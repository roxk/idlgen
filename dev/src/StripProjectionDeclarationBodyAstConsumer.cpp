#include "StripProjectionDeclarationBodyAstConsumer.h"
#include "StripProjectionDeclarationBodyVisitor.h"

idlgen::StripProjectionDeclarationBodyAstConsumer::StripProjectionDeclarationBodyAstConsumer(
    clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose
) :
    visitor(nullptr)
{
}

void idlgen::StripProjectionDeclarationBodyAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
}
