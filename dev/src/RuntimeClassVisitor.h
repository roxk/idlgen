#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/StringRef.h"
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>

namespace llvm
{
class raw_ostream;
}

namespace clang
{
class ASTContext;
class CompilerInstance;
class ClassTemplateDecl;
} // namespace clang

namespace idlgen
{
enum class IdlGenAttrType
{
    Import,
    Attribute,
    Extend,
    Hide,
    Property
};
struct IdlGenAttr
{
    IdlGenAttrType type;
    std::vector<std::string> args;
};
struct MethodGroup
{
    std::string methodName;
    clang::CXXMethodDecl* method;
    clang::CXXMethodDecl* getter;
    clang::CXXMethodDecl* setter;
    bool IsProperty() const
    {
        return getter != nullptr && setter != nullptr;
    }
    bool IsGetter() const
    {
        return getter != nullptr && setter == nullptr;
    }
    bool isMethod() const
    {
        return method != nullptr;
    }
    clang::CXXMethodDecl* getterOrElse()
    {
        return method != nullptr ? method : getter != nullptr ? getter : setter;
    }
    clang::CXXMethodDecl* setterOrElse()
    {
        return method != nullptr ? method : setter != nullptr ? setter : getter;
    }
};
enum class RuntimeClassKind
{
    Projected,
    Implementation
};
enum class EnumKind
{
    Normal,
    Flag
};
enum class MethodKind
{
    Setter,
    Getter,
    Method
};

class RuntimeClassVisitor : public clang::RecursiveASTVisitor<RuntimeClassVisitor>
{
  private:
    clang::CompilerInstance& ci;
    clang::ASTContext& astContext;
    llvm::raw_ostream& out;
    static std::unordered_map<std::string, std::string> cxxTypeToWinRtTypeMap;
    std::unordered_map<std::string, clang::CXXRecordDecl*> implementationTypes;
    bool verbose;

  public:
    explicit RuntimeClassVisitor(clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose);

    void Reset();

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* record);

    bool VisitEnumDecl(clang::EnumDecl* decl);

  private:
    std::optional<IdlGenAttr> GetIdlGenAttr(clang::Attr* attr);
    MethodGroup& GetMethodGroup(std::map<std::string, MethodGroup>& methodGroups, clang::CXXMethodDecl* method);
    void FindFileToInclude(std::set<std::string>& includes, std::string const& thisClassFileName, clang::QualType type);
    static std::unordered_map<std::string, std::string> initCxxTypeToWinRtTypeMap();
    std::string TranslateCxxTypeToWinRtType(clang::QualType type);
    static bool IsCppWinRtPrimitive(std::string const& type);
    bool IsRuntimeClassMethodType(clang::QualType type, bool projectedOnly = false);
    bool IsEventRevoker(clang::CXXMethodDecl* method);
    bool IsEventRegistrar(clang::CXXMethodDecl* method);
    bool IsConstructor(clang::CXXMethodDecl* method);
    bool IsDestructor(clang::CXXMethodDecl* method);
    bool ShouldSkipGenerating(clang::NamedDecl* decl);
    std::optional<MethodKind> GetRuntimeClassMethodKind(clang::CXXMethodDecl* method);
    static clang::QualType StripReference(clang::QualType type);
    static clang::CXXRecordDecl* StripReferenceAndGetClassDecl(clang::QualType type);
    static const clang::NamedDecl* StripReferenceAndGetNamedDecl(clang::QualType type);
    std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::QualType type);
    std::optional<EnumKind> GetEnumKind(clang::EnumDecl* decl);
    std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::CXXRecordDecl* record, bool implementationOnly = false);
    std::optional<std::vector<clang::QualType>> GetExtend(clang::CXXRecordDecl* record);
    static std::vector<std::string> GetWinRtNamespaces(clang::NamedDecl* decl);
    static std::string GetQualifiedName(clang::CXXRecordDecl* record);
    std::optional<std::string> GetLocFilePath(clang::NamedDecl* decl);
    std::string GetLocFileName(clang::CXXRecordDecl* record);
    void PrintNameSpaces(std::vector<std::string> namespaces);
    void PrintMethodParams(clang::CXXMethodDecl* method);
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is delegate</returns>
    bool TryHandleAsDelegate(clang::CXXRecordDecl* decl);
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is struct</returns>
    bool TryHandleAsStruct(clang::CXXRecordDecl* decl);
    bool IsSingleBaseOfType(clang::CXXRecordDecl* decl, std::string_view name);
    template <typename Func> void debugPrint(Func&& func)
    {
        if (verbose)
        {
            func();
        }
    }
};
} // namespace idlgen