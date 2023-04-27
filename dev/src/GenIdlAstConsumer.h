#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include <memory>

namespace llvm
{
class raw_ostream;
}

namespace idlgen
{
class RuntimeClassVisitor;

class GenIdlAstConsumer : public clang::ASTConsumer
{
  private:
    std::unique_ptr<RuntimeClassVisitor> visitor;

  public:
    explicit GenIdlAstConsumer(clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose);

    virtual void HandleTranslationUnit(clang::ASTContext& context) override;
};
} // namespace idlgen
