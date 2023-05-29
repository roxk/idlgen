#include "StripProjectionDeclarationBodyVisitor.h"
#include "clang/AST/DeclCXX.h"

idlgen::StripProjectionDeclarationBodyVisitor::StripProjectionDeclarationBodyVisitor(llvm::raw_ostream& out) : out(out)
{
}

bool idlgen::StripProjectionDeclarationBodyVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* record)
{
    return true;
}
