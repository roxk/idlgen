#include "StripProjectionDeclarationBodyAstConsumer.h"
#include "StripProjectionDeclarationBodyVisitor.h"

idlgen::StripProjectionDeclarationBodyAstConsumer::StripProjectionDeclarationBodyAstConsumer(
    clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose
) :
    visitor(std::make_unique<StripProjectionDeclarationBodyVisitor>(ci, out, verbose))
{
}

void idlgen::StripProjectionDeclarationBodyAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    visitor->Reset();
    visitor->TraverseDecl(context.getTranslationUnitDecl());
    visitor->Finish();
}
