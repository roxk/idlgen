#include "clang/AST/ASTConsumer.h"
#include "clang/Frontend/CompilerInstance.h"
#include <vector>
#include <string>

namespace llvm
{
class raw_ostream;
}

namespace idlgen
{
class GetHeaderAstConsumer : public clang::ASTConsumer
{
  private:
    clang::CompilerInstance& ci;
    std::vector<std::string>& headerFullPaths;

  public:
    explicit GetHeaderAstConsumer(clang::CompilerInstance& ci, std::vector<std::string>& headerFullPaths);

    virtual void HandleTranslationUnit(clang::ASTContext& context) override;
};
} // namespace idlgen
