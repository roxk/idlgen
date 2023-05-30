#pragma once

#include "clang/AST/RecursiveASTVisitor.h"

namespace llvm
{
class raw_ostream;
}

namespace idlgen
{

class StripProjectionDeclarationBodyVisitor : public clang::RecursiveASTVisitor<StripProjectionDeclarationBodyVisitor>
{
  private:
    llvm::raw_ostream& out;
    bool verbose{};
  public:
    StripProjectionDeclarationBodyVisitor(llvm::raw_ostream& out, bool verbose);
    void Reset();
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* record);
    bool VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl* d);
    std::string PrintTemplateParam(llvm::ArrayRef<clang::NamedDecl*> params);
    std::vector<std::string> GetNameSpaces(clang::NamedDecl* decl);
    void PrintNamespaces(clang::NamedDecl* decl);
};

} // namespace idlgen
