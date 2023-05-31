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

static lc::opt<std::string> GenerateOutputPath(
    "gen-out", lc::desc("If specified and --gen is applied, control the output path of the generated IDL")
);

static lc::opt<std::string> RootNamespace("root-namespace", lc::desc("Root namespace"));

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

void TrimIncludeStatementForFileName(std::string& includeStatement)
{
    if (auto lastQuoteIndex = includeStatement.rfind("\""))
    {
        includeStatement.erase(lastQuoteIndex, 1);
    }
    if (auto firstQuoteIndex = includeStatement.find("\""))
    {
        includeStatement.erase(0, firstQuoteIndex + 1);
    }
}

clang::FileID GetFileId(clang::SourceManager& sourceManager, std::string_view filePath)
{
    auto& fileManager{sourceManager.getFileManager()};
    clang::FileEntry const* entry{};
    if (auto fileEntry = fileManager.getFile(filePath))
    {
        entry = *fileEntry;
    }
    return sourceManager.getOrCreateFileID(entry, clang::SrcMgr::C_User);
}

llvm::StringRef GetCode(clang::SourceManager& sourceManager, std::string_view filePath)
{
    return sourceManager.getBufferData(GetFileId(sourceManager, filePath));
}

std::string StripImplementationProjectionFromHeader(
    bool verbose, std::vector<std::string> const& clangArgs, clang::StringRef file, clang::StringRef buffer
)
{
    const std::string code{buffer.str()};
    clang::LangOptions defaultLangOpt;
    constexpr auto regexStr = "^#\\s*include\\s*\"\\w*\\.g\\.h\"";
    std::regex includeRegex(regexStr);
    auto includeDirectiveMatch{std::sregex_iterator(code.begin(), code.end(), includeRegex)};
    if (includeDirectiveMatch == std::sregex_iterator())
    {
        return buffer.data();
    }
    auto firstMatchPosition{(*includeDirectiveMatch)[0].first - code.begin()};
    auto newCode{std::regex_replace(code, includeRegex, "")};
    if (newCode.size() == code.size())
    {
        return buffer.data();
    }
    // Find struct Class : ClassT<Class>
    constexpr auto baseRegexStr =
        "(struct|class)(\\s|\\w|\\[|\\]|:|\"|\\(|\\)|,|\\.|\\\\)+(\\w+)\\s+:(\\s+\\w+,*)*(\\s+\\3T)<\\3>";
    std::regex baseRegex(baseRegexStr);
    constexpr auto captureGroupCount = 5;
    constexpr auto expectedMatchCount = captureGroupCount + 1;
    constexpr auto classNameIndex = 3;
    std::string projectionReplacement{"#include <winrt/"};
    projectionReplacement += RootNamespace;
    projectionReplacement += ".h>\n";
    for (auto it = code.begin(); it != code.end();)
    {
        std::smatch results;
        if (!std::regex_search(it, code.end(), results, baseRegex) || results.size() < expectedMatchCount)
        {
            break;
        }
        it = results[0].second;
        projectionReplacement += "template <typename T, typename... I> struct ";
        projectionReplacement += results[classNameIndex];
        projectionReplacement += "T {};\n";
    }
    if (projectionReplacement.empty())
    {
        return buffer.data();
    }
    newCode.insert(firstMatchPosition, projectionReplacement);
    return newCode;
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
    if (!GeneratePch && RootNamespace.empty())
    {
        std::cerr << "Root namespace must be provided when not generating PCH" << std::endl;
        return 1;
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
                std::cerr << "Failed to create pch output dir" << std::endl;
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
            std::cerr << "Failed to get pch buffer" << std::endl;
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
                outputFile = filePath;
            }
            auto extension = llvm::sys::path::extension(filePath);
            idlFile.emplace(outputFile->get());
            if (auto extensionIndex = outputFile->get().find(extension); extensionIndex != std::string::npos)
            {
                idlFile->erase(extensionIndex);
            }
            *idlFile += ".idl";
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
        auto buffer{StripImplementationProjectionFromHeader(Verbose, clangArgs, filePath, code->getBuffer())};
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
                uint64_t genFileSize;
                if (auto ec = llvm::sys::fs::file_size(*genFile, genFileSize); !ec && genFileSize > 0)
                {
                    llvm::sys::fs::rename(*idlFile, *fileBackup);
                    llvm::sys::fs::rename(*genFile, *idlFile);
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
