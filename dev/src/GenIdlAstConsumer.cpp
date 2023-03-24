#include "GenIdlAstConsumer.h"
#include "RuntimeClassVisitor.h"

idlgen::GenIdlAstConsumer::GenIdlAstConsumer(clang::CompilerInstance& ci, llvm::StringRef fileName) :
    visitor(std::make_unique<RuntimeClassVisitor>(ci))
{
}

void idlgen::GenIdlAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    visitor->Reset();
    visitor->TraverseDecl(context.getTranslationUnitDecl());
}
