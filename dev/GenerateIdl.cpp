#include "GenIdlAstConsumer.h"
#include "RuntimeClassVisitor.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Driver/Options.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include <iostream>
#include <memory>

namespace llvm
{
class raw_ostream;
}

namespace lc = llvm::cl;
namespace ct = clang::tooling;
namespace lfs = llvm::sys::fs;

namespace idlgen
{
class GenIdlFrontendAction : public clang::ASTFrontendAction
{
  private:
    llvm::raw_ostream& out;
    bool verbose;

  public:
    GenIdlFrontendAction(llvm::raw_ostream& out, bool verbose) : out(out), verbose(verbose)
    {
    }
    std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci, clang::StringRef file) override
    {
        return std::make_unique<GenIdlAstConsumer>(ci, out, verbose);
    }
};
} // namespace idlgen

static lc::opt<bool> Help("h", lc::desc("Alias for -help"), lc::Hidden);

static lc::opt<bool> Generate("gen", lc::desc("generate IDL next to the input file"));

static lc::opt<std::string> GenerateOutputPath(
    "gen-out", lc::desc("if specified and --gen is applied, control the output path of the generated IDL")
);

static lc::list<std::string> Includes("include", lc::desc("include folder(s)"));

static lc::list<std::string> Defines("define", lc::desc("preprocessor definition(s)"));

static lc::list<std::string> FileNames(lc::Positional, lc::desc("[<file> ...]"));

static lc::opt<bool> Verbose(
    "verbose", lc::desc("Enable verbose printing for debug. Note this breaks printing into stdout")
);

static void PrintVersion(llvm::raw_ostream& OS)
{
    OS << "idlgen 0.0.1" << '\n';
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
    if (FileNames.empty())
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
    std::optional<std::reference_wrapper<std::string>> outputFile;
    if (Generate && !GenerateOutputPath.empty())
    {
        outputFile = GenerateOutputPath;
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
        auto buffer{code->getBuffer()};
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
        const auto result = ct::runToolOnCodeWithArgs(
            std::make_unique<idlgen::GenIdlFrontendAction>(out.value().get(), Verbose), buffer, clangArgs, filePath
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
