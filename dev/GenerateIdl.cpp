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

static lc::opt<bool> GenerateBootstrap("gen-bootstrap", lc::desc("Generate bootstrap idl"));

static lc::opt<bool> GenerateProjectionPatch(
    "gen-projection-patch",
    lc::desc("Generate projection patch, such as IClassProtected. Expect input file to be Class.g.h")
);

static lc::opt<std::string> GenerateOutputPath(
    "gen-out", lc::desc("If specified and --gen is applied, control the output path of the generated IDL")
);

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

template <typename Func> void DebugPrint(Func&& func)
{
    if (Verbose)
    {
        func();
    }
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

struct IdlWriter
{
    std::unique_ptr<llvm::raw_ostream> out;
    IdlWriter(IdlWriter&& that) = default;
    IdlWriter& operator=(IdlWriter&& that) = default;
    ~IdlWriter()
    {
        if (!out)
        {
            return;
        }
        out.reset();
        std::string fileBackup{idlFile + ".bak"};
        std::string genFile{idlFile + ".gen"};
        stdfs::path genFilePath{genFile};
        const uint64_t genFileSize{stdfs::file_size(genFilePath)};
        stdfs::path idlFilePath{idlFile};
        if (genFileSize > 0)
        {
            if (stdfs::exists(idlFilePath))
            {
                const uint64_t idlFileSize{stdfs::file_size(idlFilePath)};
                if (genFileSize != idlFileSize || !IsContentEqual(genFilePath, idlFilePath))
                {
                    llvm::sys::fs::rename(idlFile, fileBackup);
                    llvm::sys::fs::rename(genFile, idlFile);
                }
            }
            else
            {
                llvm::sys::fs::rename(idlFile, fileBackup);
                llvm::sys::fs::rename(genFile, idlFile);
            }
        }
        lfs::remove(genFile);
    }

  private:
    std::string idlFile;
    IdlWriter(std::unique_ptr<llvm::raw_ostream> out, std::string idlFile) :
        out(std::move(out)),
        idlFile(std::move(idlFile))
    {
    }
    friend std::optional<IdlWriter> GetIdlWriter(std::string_view filePath, bool replaceExtension);
};

std::string GetIdl(std::string_view path)
{
    // Trim all extension to handle cases like .xaml.h -> .idl
    std::string idlFile{path};
    auto extension = llvm::sys::path::extension(idlFile);
    auto extensionIndex = idlFile.rfind(extension);
    while (extensionIndex != std::string::npos && extensionIndex < idlFile.size())
    {
        idlFile.erase(extensionIndex);
        extension = llvm::sys::path::extension(idlFile);
        extensionIndex = idlFile.rfind(extension);
    }
    idlFile += ".idl";
    return idlFile;
}

std::optional<IdlWriter> GetIdlWriter(std::string_view filePath, bool replaceExtension)
{
    std::string idlFile{replaceExtension ? GetIdl(filePath) : filePath};
    std::error_code ec;
    auto out{std::make_unique<llvm::raw_fd_ostream>(
        idlFile + ".gen",
        ec,
        lfs::CreationDisposition::CD_CreateAlways,
        lfs::FileAccess::FA_Write,
        lfs::OpenFlags::OF_None
    )};
    if (ec)
    {
        std::cerr << "Failed to open gen output for " << idlFile << std::endl;
        std::cerr << ec.message() << std::endl;
        return std::nullopt;
    }
    return IdlWriter(std::move(out), std::move(idlFile));
}

// Pasting for easier copy and paste.
// TODO: Use constexpr std::string in C++20.
constexpr auto NameRegexMiddle =
    "\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|:|\\.)*\\]\\]\\s+)*_*(\\w+)\\s*:\\s*(idlgen::)*";

template <int CaptureGroupCount, int NameIndex, typename StringType>
std::set<std::string> FindAuthoredTypeNames(const std::string& code, StringType&& regexStr)
{
    std::regex regex(regexStr);
    constexpr auto expectedMatchCount = CaptureGroupCount + 1;
    std::set<std::string> names;
    for (auto it = code.begin(); it != code.end();)
    {
        std::smatch results;
        if (!std::regex_search(it, code.end(), results, regex) || results.size() < expectedMatchCount)
        {
            break;
        }
        it = results[0].second;
        names.insert(results[NameIndex]);
    }
    return names;
}

std::set<std::string> FindRuntimeClassNames(const std::string& code)
{
    constexpr auto regexStr =
        "(struct|class)\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|_|:|\\.)*\\]\\]\\s+)*(\\w+)\\s*:\\s*.+\\s*"
        "(idlgen::)*author_class";
    std::regex regex(regexStr);
    constexpr auto captureGroupCount = 5;
    constexpr auto expectedMatchCount = captureGroupCount + 1;
    constexpr auto classNameIndex = 4;
    std::set<std::string> classNames;
    for (auto it = code.begin(); it != code.end();)
    {
        std::smatch results;
        if (!std::regex_search(it, code.end(), results, regex) || results.size() < expectedMatchCount)
        {
            break;
        }
        it = results[0].second;
        classNames.insert(results[classNameIndex]);
    }
    return classNames;
}

std::set<std::string> FindEnumNames(const std::string& code)
{
    constexpr auto regexStr = "enum\\s+class\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|:|\\.)*\\]\\]\\s+)*_*(\\w+)\\s*:\\s*("
                              "idlgen::)*author_enum(_flags)*";
    return FindAuthoredTypeNames<5, 3>(code, regexStr);
}

std::set<std::string> FindStructNames(const std::string& code)
{
    constexpr auto regexStr = "(struct|class)\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|:|\\.)*\\]\\]\\s+)*_*(\\w+)\\s*:\\s*"
                              "(idlgen::)*author_struct";
    return FindAuthoredTypeNames<5, 4>(code, regexStr);
}

std::set<std::string> FindInterfaceNames(const std::string& code)
{
    constexpr auto regexStr = "(struct|class)\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|:|\\.)*\\]\\]\\s+)*_*(\\w+)\\s*:\\s*"
                              "(idlgen::)*author_interface";
    return FindAuthoredTypeNames<5, 4>(code, regexStr);
}

std::set<std::string> FindDelegateNames(const std::string& code)
{
    constexpr auto regexStr = "(struct|class)\\s+(\\[\\[(\\w|\\s|\\(|\\)|\"|\\\\|,|:|\\.)*\\]\\]\\s+)*_*(\\w+)\\s*:\\s*"
                              "(idlgen::)*author_delegate";
    return FindAuthoredTypeNames<5, 4>(code, regexStr);
}

/// <summary>
/// Given winrt::A::implementation, returns winrt::A.
/// </summary>
/// <param name="code"></param>
/// <returns></returns>
std::string FindWinRtNamespaceInCpp(std::string const& code, bool removeWinRt)
{
    // Find namespace
    constexpr auto namespaceStr = "namespace\\s+(\\w|::|)+\\s*\\{";
    std::regex namespaceRegex(namespaceStr);
    std::smatch namespaceResults;
    constexpr auto namespaceIndex = 0;
    if (!std::regex_search(code.begin(), code.end(), namespaceResults, namespaceRegex))
    {
        return "";
    }
    // Get WinRT namespace in C++
    auto namespaceMatchResult{namespaceResults[namespaceIndex]};
    return std::regex_replace(
        namespaceMatchResult.str(),
        std::regex(
            removeWinRt ? "(::implementation|winrt::|::factory_implementation)"
                        : "(::implementation|::factory_implementation)"
        ),
        ""
    );
}

bool GenerateBootstrapIdl(std::string_view filePath, clang::StringRef buffer)
{
    const std::string code{buffer.str()};
    std::string namespaceDefinition{FindWinRtNamespaceInCpp(code, true)};
    if (namespaceDefinition.empty())
    {
        return true;
    }
    namespaceDefinition = std::regex_replace(namespaceDefinition, std::regex("::"), ".");
    auto classNames{FindRuntimeClassNames(code)};
    auto enumNames{FindEnumNames(code)};
    auto structNames{FindStructNames(code)};
    auto interfaceNames{FindInterfaceNames(code)};
    auto delegateNames{FindDelegateNames(code)};
    std::string bootstrapIdl{namespaceDefinition};
    bootstrapIdl += "\n";
    const auto idlSize = bootstrapIdl.size();
    for (auto&& name : enumNames)
    {
        bootstrapIdl += "enum ";
        bootstrapIdl += name;
        bootstrapIdl += " { _Dummy };\n";
    }
    for (auto&& name : classNames)
    {
        bootstrapIdl += "[default_interface] runtimeclass ";
        bootstrapIdl += name;
        bootstrapIdl += " : Windows.UI.Xaml.DependencyObject { ";
        bootstrapIdl += name;
        bootstrapIdl += "(); };\n";
    }
    for (auto&& name : structNames)
    {
        bootstrapIdl += "struct ";
        bootstrapIdl += name;
        bootstrapIdl += " { Int32 _Dummy; };\n";
    }
    for (auto&& name : interfaceNames)
    {
        bootstrapIdl += "interface ";
        bootstrapIdl += name;
        bootstrapIdl += " {};\n";
    }
    for (auto&& name : delegateNames)
    {
        bootstrapIdl += "delegate void ";
        bootstrapIdl += name;
        bootstrapIdl += "(Object a, Object b);\n";
    }
    const auto isEmpty = bootstrapIdl.size() == idlSize;
    if (isEmpty && !stdfs::exists(GetIdl(filePath)))
    {
        // If content is empty (i.e. No WinRT entities) and idl file doesn't exist, the file
        // is 99.9% not an implementation type header. Do nothing.
        // Note that we still want to output an empty idl in case an existing idl file is present
        // since the empty content might be caused by refactoring an implementation type header
        // or editing mistakes.
        return true;
    }
    bootstrapIdl += "}\n"; // namespace $RootNamespace
    auto writerOpt{GetIdlWriter(filePath, true)};
    if (!writerOpt)
    {
        std::cerr << "Failed to get idl writer" << std::endl;
        return false;
    }
    auto writer{std::move(*writerOpt)};
    DebugPrint(
        [&]()
        {
            std::cout << "Generating bootstrap idl" << std::endl;
            std::cout << filePath << std::endl;
            std::cout << bootstrapIdl << std::endl;
        }
    );
    *writer.out << bootstrapIdl;
    return true;
}

bool GenerateProjectionPatchToGeneratedHeader(clang::StringRef filePath, clang::StringRef buffer)
{
    std::string code{buffer};
    auto fileName{lsp::filename(filePath)};
    auto className{std::regex_replace(fileName.str(), std::regex(".g.h"), "")};
    DebugPrint([&]() { std::cout << "Generating projection patch for class " << className << std::endl; });
    auto namespaceDefinition{FindWinRtNamespaceInCpp(code, false)};
    if (namespaceDefinition.empty())
    {
        DebugPrint([]() { std::cout << "No namespce to insert forward declaration" << std::endl; });
        return true;
    }
    std::string insertion{namespaceDefinition};
    insertion += "\n";
    insertion += "struct I";
    insertion += className;
    insertion += "Protected;\n";
    insertion += "struct I";
    insertion += className;
    insertion += "Statics;\n";
    insertion += "}\n"; // namespace $RootNamespace
    DebugPrint(
        [&]()
        {
            std::cout << "Inserting forward declaration for class interfaces" << std::endl;
            std::cout << insertion << std::endl;
        }
    );
    std::error_code ec;
    auto out{std::make_unique<llvm::raw_fd_ostream>(
        filePath, ec, lfs::CreationDisposition::CD_OpenExisting, lfs::FileAccess::FA_Write, lfs::OpenFlags::OF_Append
    )};
    if (ec)
    {
        std::cerr << "Failed to open output for " << filePath.str() << std::endl;
        std::cerr << ec.message() << std::endl;
        return false;
    }
    *out << insertion;
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
    if (GenerateBootstrap)
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
            const auto result{GenerateBootstrapIdl(filePath, code->getBuffer())};
            if (!result)
            {
                std::cerr << "fatal: Failed to generate fake projection for " << filePath << std::endl;
                return 1;
            }
        }
        return 0;
    }
    if (GenerateProjectionPatch)
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
            const auto result{GenerateProjectionPatchToGeneratedHeader(filePath, code->getBuffer())};
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
        std::optional<IdlWriter> writerOpt;
        auto out{
            [&]() -> std::optional<std::reference_wrapper<llvm::raw_ostream>>
            {
                if (!Generate)
                {
                    return llvm::outs();
                }
                if (!outputFile)
                {
                    writerOpt = std::move(GetIdlWriter(filePath, true));
                }
                else
                {
                    writerOpt = std::move(GetIdlWriter(outputFile->get(), false));
                }
                if (!writerOpt)
                {
                    return std::nullopt;
                }
                return *writerOpt->out;
            }()};
        if (!out)
        {
            return 1;
        }
        const auto result = ct::runToolOnCodeWithArgs(
            std::make_unique<idlgen::GenIdlFrontendAction>(
                out.value().get(), Verbose, getterTemplates, propertyTemplates
            ),
            code->getBuffer(),
            clangArgs,
            filePath
        );
        if (Generate)
        {
            // Flush the file ostream
            writerOpt.reset();
        }
        outputFile.reset();
    }
    return 0;
}
