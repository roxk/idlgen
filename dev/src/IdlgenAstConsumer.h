#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>
#include <string>

namespace llvm
{
class raw_ostream;
}

namespace idlgen
{
class IdlgenVisitor;

class IdlgenAstConsumer : public clang::ASTConsumer
{
  private:
    std::unique_ptr<IdlgenVisitor> visitor;

  public:
    explicit IdlgenAstConsumer(
        clang::CompilerInstance& ci,
        llvm::raw_ostream& out,
        bool verbose,
        std::vector<std::string> const& getterTemplates,
        std::vector<std::string> const& propertyTemplates
    );

    virtual void HandleTranslationUnit(clang::ASTContext& context) override;
};
} // namespace idlgen
