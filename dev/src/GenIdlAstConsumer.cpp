#include "GenIdlAstConsumer.h"
#include "RuntimeClassVisitor.h"

idlgen::GenIdlAstConsumer::GenIdlAstConsumer(
    clang::CompilerInstance& ci,
    llvm::raw_ostream& out,
    bool verbose,
    std::vector<std::string> const& getterTemplates,
    std::vector<std::string> const& propertyTemplates
) :
    visitor(std::make_unique<RuntimeClassVisitor>(ci, out, verbose, getterTemplates, propertyTemplates))
{
}

void idlgen::GenIdlAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    visitor->Reset();
    visitor->TraverseDecl(context.getTranslationUnitDecl());
}
