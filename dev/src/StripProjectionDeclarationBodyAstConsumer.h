#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>

namespace llvm
{
class raw_ostream;
}

namespace idlgen
{
class StripProjectionDeclarationBodyVisitor;

class StripProjectionDeclarationBodyAstConsumer : public clang::ASTConsumer
{
  private:
    std::unique_ptr<StripProjectionDeclarationBodyVisitor> visitor;

  public:
    explicit StripProjectionDeclarationBodyAstConsumer(
        clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose
    );

    virtual void HandleTranslationUnit(clang::ASTContext& context) override;
};
} // namespace idlgen
