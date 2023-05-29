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
  public:
    StripProjectionDeclarationBodyVisitor(llvm::raw_ostream& out);
    bool VisitCXXRecordDecl(clang::CXXRecordDecl* record);
};

} // namespace idlgen
