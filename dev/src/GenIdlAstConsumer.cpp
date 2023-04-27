#include "GenIdlAstConsumer.h"
#include "RuntimeClassVisitor.h"

idlgen::GenIdlAstConsumer::GenIdlAstConsumer(clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose)
    : visitor(std::make_unique<RuntimeClassVisitor>(ci, out, verbose))
{
}

void idlgen::GenIdlAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    visitor->Reset();
    visitor->TraverseDecl(context.getTranslationUnitDecl());
}
