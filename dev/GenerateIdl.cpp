#include "clang/Driver/Options.h"
#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/ADT/StringRef.h"
#include "GenIdlAstConsumer.h"
#include "RuntimeClassVisitor.h"
#include <memory>
#include <iostream>

namespace lc = llvm::cl;
namespace ct = clang::tooling;

namespace idlgen
{
    class GenIdlFrontendAction : public clang::ASTFrontendAction
    {
    private:
        llvm::StringRef fileName;
    public:
        GenIdlFrontendAction(llvm::StringRef fileName) : fileName(fileName) {}
        std::unique_ptr<clang::ASTConsumer> CreateASTConsumer(clang::CompilerInstance& ci, clang::StringRef file) override
        {
            return std::make_unique<GenIdlAstConsumer>(ci, fileName);
        }
    };
}

static lc::opt<bool> Help("h", lc::desc("Alias for -help"), lc::Hidden);

static lc::list<std::string> Includes("include", lc::desc("include folder(s)"));

static lc::list<std::string> FileNames(lc::Positional, lc::desc("[<file> ...]"));

static void PrintVersion(llvm::raw_ostream& OS)
{
    OS << "idlgen 0.0.1" << '\n';
}

int main(int argc, const char** argv)
{
    lc::SetVersionPrinter(PrintVersion);
    lc::ParseCommandLineOptions(
        argc, argv,
        "A tool to generate .idl from a C++ header file.");
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
    std::vector<std::string> clangArgs
    {
        "-xc++-header", 
        "-Wno-unknown-attributes",
        // winrt/base.h QueryInterface etc no override
        "-Wno-inconsistent-missing-override",
        "-std=c++20",
        "-DWIN32_LEAN_AND_MEAN", 
        "-DWINRT_LEAN_AND_MEAN", 
        "-D_Windows",
        "-D_UNICODE", 
        "-DUNICODE", 
        "-DWINAPI_FAMILY=WINAPI_FAMILY_APP"
    };
    for (auto&& include : Includes)
    {
        clangArgs.emplace_back("-I" + include);
    }
    for (auto&& filePath : FileNames)
    {
        std::cout << "Generating idl for " << filePath << std::endl;
        auto codeOrErr = llvm::MemoryBuffer::getFileAsStream(filePath);
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
        auto buffer = code->getBuffer();
        auto fileName = llvm::sys::path::filename(filePath);
        ct::runToolOnCodeWithArgs(std::make_unique<idlgen::GenIdlFrontendAction>(fileName), buffer, clangArgs, filePath);
    }
    return 0;
}
