#include "IdlgenAstConsumer.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/CodeGen/ObjectFilePCHContainerOperations.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/FrontendActions.h"
#include "clang/Lex/HeaderSearchOptions.h"
#include "clang/Lex/PreprocessorOptions.h"
#include "clang/Rewrite/Core/Rewriter.h"
#include "clang/Serialization/InMemoryModuleCache.h"
#include "clang/Tooling/Core/Replacement.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <string>

namespace llvm
{
class raw_ostream;
}

namespace lc = llvm::cl;
namespace ct = clang::tooling;
namespace lfs = llvm::sys::fs;
namespace lsp = llvm::sys::path;
namespace stdfs = std::filesystem;

namespace idlgen
{
class GenIdlFrontendAction : public clang::ASTFrontendAction
{
  private:
    llvm::raw_ostream& out;
    bool verbose;
    std::vector<std::string> const& getterTemplates;
    std::vector<std::string> const& propertyTemplates;

  public:
    GenIdlFrontendAction(
        llvm::raw_ostream& out,
        bool verbose,
        std::vector<std::string> const& getterTemplates,
        std::vector<std::string> const& propertyTemplates
    ) :
        out(out),
        verbose(verbose),
        getterTemplates(getterTemplates),
        propertyTemplates(propertyTemplates)
    {
    }
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci, clang::StringRef file) override
    {
        return std::make_unique<IdlgenAstConsumer>(ci, out, verbose, getterTemplates, propertyTemplates);
    }
};
} // namespace idlgen

class GeneratePchActionWrapper : public clang::GeneratePCHAction
{
  private:
    std::string output;

  public:
    GeneratePchActionWrapper(std::string output) : output(std::move(output))
    {
    }
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci, clang::StringRef file) override
    {
        ci.getFrontendOpts().OutputFile = output;
        return clang::GeneratePCHAction::CreateASTConsumer(ci, file);
    }
};

static lc::opt<bool> Help("h", lc::desc("Alias for -help"), lc::Hidden);

static lc::opt<bool> Generate("gen", lc::desc("Generate IDL next to the input file"));

static lc::opt<bool> GenerateFakeProjection(
    "gen-fake-projection", lc::desc("Generate fake projection. --generated-files-dir and --root-namespace is required.")
);

static lc::opt<std::string> GenerateOutputPath(
    "gen-out", lc::desc("If specified and --gen is applied, control the output path of the generated IDL")
);

static lc::opt<std::string> RootNamespace("root-namespace", lc::desc("Root namespace"));

static lc::opt<std::string> GeneratedFilesDir("--generated-files-dir", lc::desc("Path of Generated Files"));

static lc::list<std::string> Includes("include", lc::desc("Include folder(s)"));

static lc::list<std::string> Defines("define", lc::desc("Preprocessor definition(s)"));

static lc::list<std::string> GetterTemplates(
    "getter-template", lc::desc("Qualified name of templates that should be treated as a getter")
);

static lc::list<std::string> PropertyTemplates(
    "property-template", lc::desc("Qualified name of templates that should be treated as a property")
);

static lc::list<std::string> FileNames(lc::Positional, lc::desc("[<file> ...]"));

static lc::opt<bool> Verbose(
    "verbose", lc::desc("Enable verbose printing for debug. Note this breaks printing into stdout")
);

static lc::opt<std::string> Pch("pch", lc::desc("Pch files"));

static lc::opt<std::string> PchOutDir("pch-out-dir", lc::desc("Directory for pch output"));

static lc::opt<bool> GeneratePch("gen-pch", lc::desc("Generate pch"));

static void PrintVersion(llvm::raw_ostream& OS)
{
    OS << "idlgen 0.0.1" << '\n';
}

bool GenerateFakeProjectionFromHeader(
    clang::StringRef buffer
)
{
    const std::string code{buffer.str()};
    constexpr auto regexStr = "^#\\s*include\\s*\"\\w*\\.g\\.h\"";
    std::regex includeRegex(regexStr);
    auto includeDirectiveMatch{std::sregex_iterator(code.begin(), code.end(), includeRegex)};
    if (includeDirectiveMatch == std::sregex_iterator())
    {
        return true;
    }
    auto firstMatchPosition{(*includeDirectiveMatch)[0].first - code.begin()};
    // Find namespace
    constexpr auto namespaceStr = "namespace\\s+(\\w|::|)+\\s*\\{";
    std::regex namespaceRegex(namespaceStr);
    std::smatch namespaceResults;
    constexpr auto namespaceIndex = 0;
    if (!std::regex_search(code.begin(), code.end(), namespaceResults, namespaceRegex))
    {
        return true;
    }
    // "Canonicalize" to WinRT namespace, i.e. remove implementation or factory_implementation
    // Using :_: as this is an invalid C++ token
    auto namespaceMatchResult{namespaceResults[namespaceIndex]};
    std::string namespaceDefinition{
        std::regex_replace(namespaceMatchResult.str(), std::regex("::implementation"), ":_:")};
    if (namespaceDefinition.size() == namespaceMatchResult.length())
    {
        namespaceDefinition = std::regex_replace(namespaceDefinition, std::regex("::factory_implementation"), ":_:");
    }
    // Find struct Class : ClassT<Class>
    constexpr auto baseRegexStr =
        "(struct|class)(\\s|\\w|\\[|\\]|:|\"|\\(|\\)|,|\\.|\\\\)+(\\w+)\\s+:(\\s+\\w+,*)*(\\s+\\3T)<\\3>";
    std::regex baseRegex(baseRegexStr);
    constexpr auto baseCaptureGroupCount = 5;
    constexpr auto expectedBaseMatchCount = baseCaptureGroupCount + 1;
    constexpr auto classNameIndex = 3;
    std::vector<std::string> classNames;
    for (auto it = code.begin(); it != code.end();)
    {
        std::smatch results;
        if (!std::regex_search(it, code.end(), results, baseRegex) || results.size() < expectedBaseMatchCount)
        {
            break;
        }
        it = results[0].second;
        classNames.emplace_back(results[classNameIndex]);
    }
    if (classNames.empty())
    {
        return buffer.data();
    }
    for (auto&& name : classNames)
    {
        std::string projectionReplacement{"#pragma once\n\n"};
        projectionReplacement += "#include <winrt/";
        projectionReplacement += RootNamespace;
        projectionReplacement += ".h>\n";
        projectionReplacement += std::regex_replace(namespaceDefinition, std::regex(":_:"), "");
        projectionReplacement += "\n";
        projectionReplacement += "struct ";
        projectionReplacement += name;
        projectionReplacement += " {};\n";
        projectionReplacement += "}\n"; // namespace $RootNamespace
        projectionReplacement += std::regex_replace(namespaceDefinition, std::regex(":_:"), "::implementation");
        projectionReplacement += "\n";
        projectionReplacement += "template <typename T, typename... I> struct ";
        projectionReplacement += name;
        projectionReplacement += "T {};\n";
        projectionReplacement += "}\n"; // namespace implementation
        projectionReplacement += std::regex_replace(namespaceDefinition, std::regex(":_:"), "::factory_implementation");
        projectionReplacement += "\n";
        projectionReplacement += "template <typename T, typename... I> struct ";
        projectionReplacement += name;
        projectionReplacement += "T {};\n";
        projectionReplacement += "}\n"; // namespace factory_implementation
        std::cout << projectionReplacement << std::endl;
        std::error_code ec;
        stdfs::path outPath{GeneratedFilesDir.getValue()};
        outPath.append("");
        llvm::raw_fd_ostream out(
            outPath.string(), ec, lfs::CreationDisposition::CD_CreateAlways, lfs::FileAccess::FA_Write, lfs::OpenFlags::OF_None
        );
        if (ec)
        {
            std::cerr << "Failed to open .idlgen.h output" << std::endl;
            std::cerr << ec.message() << std::endl;
            return false;
        }
        out << projectionReplacement;
    }
    return true;
}

/// <summary>
/// Assumes file size is the same.
/// </summary>
/// <param name="lhs"></param>
/// <param name="rhs"></param>
/// <returns></returns>
bool IsContentEqual(std::filesystem::path& lhs, std::filesystem::path& rhs)
{
    constexpr uint64_t blockSize = 4096;
    uint64_t remainderFileSize = std::filesystem::file_size(lhs);
    std::ifstream lhsStream{lhs};
    std::ifstream rhsStream{rhs};
    while (true)
    {
        char buffer1[blockSize];
        char buffer2[blockSize];
        size_t size = std::min(blockSize, remainderFileSize);
        lhsStream.read(buffer1, size);
        rhsStream.read(buffer2, size);
        if (memcmp(buffer1, buffer2, size) != 0)
        {
            return false;
        }
        if (remainderFileSize > size)
        {
            remainderFileSize -= size;
        }
        else
        {
            break;
        }
    }
    return true;
}

int main(int argc, const char** argv)
{
    lc::SetVersionPrinter(PrintVersion);
    lc::ParseCommandLineOptions(argc, argv, "A tool to generate .idl from a C++ header file.");
    if (Help)
    {
        lc::PrintHelpMessage();
        return 0;
    }
    if ((GeneratePch && Pch.empty()) || (!GeneratePch && FileNames.empty()))
    {
        std::cerr << "No files specified" << std::endl;
        return 1;
    }
    if (GenerateFakeProjection)
    {
        if (RootNamespace.empty())
        {
            std::cerr << "Root namespace must be provided when generating fake projection" << std::endl;
            return 1;
        }
        else if (GeneratedFilesDir.empty())
        {
            std::cerr << "Generated Files must be provided when generating fake projection" << std::endl;
            return 1;
        }
    }
    std::vector<std::string> clangArgs{
        "-xc++-header",
        "-Wno-unknown-attributes",
        // winrt/base.h QueryInterface etc no override
        "-Wno-inconsistent-missing-override",
        // Xaml compiler use windows.foundation instead of Windows.Foundation
        "-Wno-nonportable-include-path",
        "-std=c++20",
        "-DWIN32_LEAN_AND_MEAN",
        "-DWINRT_LEAN_AND_MEAN",
        "-D_Windows",
        "-D_UNICODE",
        "-DUNICODE",
        "-DWINAPI_FAMILY=WINAPI_FAMILY_APP",
    };
    for (auto&& include : Includes)
    {
        clangArgs.emplace_back("-I" + include);
    }
    for (auto&& define : Defines)
    {
        clangArgs.emplace_back("-D" + define);
    }
    if (FileNames.size() > 1 && Generate && GenerateOutputPath.hasArgStr())
    {
        std::cerr << "gen-out is specified with more than 1 input file. "
                     "Please only specify 1 input file if gen-out is specified"
                  << std::endl;
        return 1;
    }
    if (GenerateFakeProjection)
    {
        for (auto&& filePath : FileNames)
        {
            auto codeOrErr{llvm::MemoryBuffer::getFileAsStream(filePath)};
            if (std::error_code ec = codeOrErr.getError())
            {
                std::cerr << ec.message() << std::endl;
                return 1;
            }
            auto code(std::move(codeOrErr.get()));
            if (code->getBufferSize() == 0)
            {
                continue;
            }
            const auto result{GenerateFakeProjectionFromHeader(code->getBuffer())};
            if (!result)
            {
                std::cerr << "fatal: Failed to generate fake projection for " << filePath << std::endl;
                return 1;
            }
        }
        return 0;
    }
    std::vector<std::string> getterTemplates(GetterTemplates);
    std::vector<std::string> propertyTemplates(PropertyTemplates);
    std::optional<std::reference_wrapper<std::string>> outputFile;
    if (Generate && !GenerateOutputPath.empty())
    {
        outputFile = GenerateOutputPath;
    }
    auto pchOutGetter = [&](std::string const& pch)
    {
        std::filesystem::path outFile{PchOutDir.c_str()};
        outFile.append(pch + ".gch");
        return outFile.string();
    };
    if (GeneratePch)
    {
        if (!lfs::exists(PchOutDir))
        {
            auto ec{lfs::create_directories(PchOutDir.c_str())};
            if (ec)
            {
                std::cerr << "fatal: Failed to create pch output dir" << std::endl;
                return 1;
            }
        }
        auto pch{Pch.getValue()};
        auto outFile{pchOutGetter(pch)};
        auto codeOrErr{llvm::MemoryBuffer::getFileAsStream(pch)};
        if (std::error_code ec = codeOrErr.getError())
        {
            std::cerr << ec.message() << std::endl;
            return 1;
        }
        auto code(std::move(codeOrErr.get()));
        if (code->getBufferSize() == 0)
        {
            std::cerr << "fatal: Failed to get pch buffer" << std::endl;
            return 1;
        }
        auto buffer{code->getBuffer()};
        auto fileName{llvm::sys::path::filename(pch).str()};
        auto result{ct::runToolOnCodeWithArgs(
            std::make_unique<GeneratePchActionWrapper>(std::move(outFile)), buffer, clangArgs, pch
        )};
        if (!result)
        {
            std::cerr << "fatal: Failed to generate pch" << std::endl;
            return 1;
        }
    }
    if (!Pch.empty())
    {
        auto pch{Pch.getValue()};
        auto outFile{pchOutGetter(pch)};
        clangArgs.emplace_back("-include-pch");
        clangArgs.emplace_back(outFile);
    }
    for (auto&& filePath : FileNames)
    {
        auto codeOrErr{llvm::MemoryBuffer::getFileAsStream(filePath)};
        if (std::error_code ec = codeOrErr.getError())
        {
            std::cerr << ec.message() << std::endl;
            return 1;
        }
        auto code(std::move(codeOrErr.get()));
        if (code->getBufferSize() == 0)
        {
            continue;
        }
        auto fileName{llvm::sys::path::filename(filePath).str()};
        std::optional<std::string> fileBackup;
        std::optional<std::string> genFile;
        std::optional<std::string> idlFile;
        std::optional<llvm::raw_fd_ostream> fileOutputStreamOpt;
        auto out = [&]() -> std::optional<std::reference_wrapper<llvm::raw_ostream>>
        {
            if (!Generate)
            {
                return llvm::outs();
            }
            if (!outputFile)
            {
                idlFile.emplace(filePath);
                // Trim all extension to handle cases like .xaml.h -> .idl
                auto extension = llvm::sys::path::extension(*idlFile);
                auto extensionIndex = idlFile->rfind(extension);
                while (extensionIndex != std::string::npos && extensionIndex < idlFile->size())
                {
                    idlFile->erase(extensionIndex);
                    extension = llvm::sys::path::extension(*idlFile);
                    extensionIndex = idlFile->rfind(extension);
                }
                *idlFile += ".idl";
            }
            else
            {
                idlFile.emplace(outputFile->get());
            }
            fileBackup = *idlFile + ".bak";
            genFile = *idlFile + ".gen";
            std::error_code ec;
            fileOutputStreamOpt.emplace(
                *genFile,
                ec,
                lfs::CreationDisposition::CD_CreateAlways,
                lfs::FileAccess::FA_Write,
                lfs::OpenFlags::OF_None
            );
            if (ec)
            {
                std::cerr << "fatal: Failed to open default idl output" << std::endl;
                std::cerr << ec.message() << std::endl;
                return std::nullopt;
            }
            return *fileOutputStreamOpt;
        }();
        if (!out)
        {
            return 1;
        }
        auto buffer{code->getBuffer()};
        const auto result = ct::runToolOnCodeWithArgs(
            std::make_unique<idlgen::GenIdlFrontendAction>(
                out.value().get(), Verbose, getterTemplates, propertyTemplates
            ),
            buffer,
            clangArgs,
            filePath
        );
        if (Generate)
        {
            // Flush the file ostream
            fileOutputStreamOpt.reset();
            if (result && fileBackup && genFile && idlFile)
            {
                namespace stdfs = std::filesystem;
                stdfs::path genFilePath{*genFile};
                const uint64_t genFileSize{stdfs::file_size(genFilePath)};
                if (genFileSize > 0)
                {
                    stdfs::path idlFilePath{*idlFile};
                    if (stdfs::exists(idlFilePath))
                    {
                        const uint64_t idlFileSize{stdfs::file_size(idlFilePath)};
                        if (genFileSize != idlFileSize || !IsContentEqual(genFilePath, idlFilePath))
                        {
                            llvm::sys::fs::rename(*idlFile, *fileBackup);
                            llvm::sys::fs::rename(*genFile, *idlFile);
                        }
                    }
                    else
                    {
                        llvm::sys::fs::rename(*idlFile, *fileBackup);
                        llvm::sys::fs::rename(*genFile, *idlFile);
                    }
                }
            }
            if (genFile)
            {
                llvm::sys::fs::remove(*genFile);
            }
        }
        outputFile.reset();
    }
    return 0;
}
