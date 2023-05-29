#include "IdlgenAstConsumer.h"
#include "IdlgenVisitor.h"

idlgen::IdlgenAstConsumer::IdlgenAstConsumer(
    clang::CompilerInstance& ci,
    llvm::raw_ostream& out,
    bool verbose,
    std::vector<std::string> const& getterTemplates,
    std::vector<std::string> const& propertyTemplates
) :
    visitor(std::make_unique<IdlgenVisitor>(ci, out, verbose, getterTemplates, propertyTemplates))
{
}

void idlgen::IdlgenAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    visitor->Reset();
    visitor->TraverseDecl(context.getTranslationUnitDecl());
}
