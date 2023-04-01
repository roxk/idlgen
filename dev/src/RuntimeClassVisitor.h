#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/StringRef.h"
#include <memory>
#include <vector>
#include <map>
#include <unordered_map>
#include <optional>
#include <set>

namespace llvm
{
    class raw_ostream;
}

namespace clang
{
    class ASTContext;
    class CompilerInstance;
    class ClassTemplateDecl;
}

namespace idlgen
{
    enum class IdlGenAttrType
    {
        Import,
        Attribute,
        Extend
    };
    struct IdlGenAttr
    {
        IdlGenAttrType type;
        std::vector<std::string> args;
    };
    struct MethodGroup
    {
        clang::CXXMethodDecl* method;
        clang::CXXMethodDecl* getter;
        clang::CXXMethodDecl* setter;
        bool IsProperty() const { return getter != nullptr && setter != nullptr; }
        bool IsGetter() const { return getter != nullptr && setter == nullptr; }
        bool isMethod() const { return method != nullptr; }
        clang::CXXMethodDecl* getterOrElse()
        {
            return method != nullptr
                ? method
                : getter != nullptr
                ? getter
                : setter;
        }
        clang::CXXMethodDecl* setterOrElse()
        {
            return method != nullptr
                ? method
                : setter != nullptr
                ? setter
                : getter;
        }
    };
    enum class RuntimeClassKind
    {
        Projected,
        Implementation
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
        clang::ASTContext& astContext;
        llvm::raw_ostream& out;
        static std::unordered_map<std::string, std::string> cxxTypeToWinRtTypeMap;
        std::unordered_map<std::string, clang::CXXRecordDecl*> implementationTypes;

    public:
        explicit RuntimeClassVisitor(clang::CompilerInstance& ci, llvm::raw_ostream& out);

        void Reset();

        bool VisitCXXRecordDecl(clang::CXXRecordDecl* record);
    private:
        static std::optional<IdlGenAttr> GetIdlGenAttr(clang::Attr* attr);
        static MethodGroup& GetMethodGroup(std::map<std::string, MethodGroup>& methodGroups, std::string const& methodName);
        void FindFileToInclude(std::set<std::string>& includes, std::string const& thisClassFileName, clang::QualType type);
        static std::unordered_map<std::string, std::string> initCxxTypeToWinRtTypeMap();
        static std::string TranslateCxxTypeToWinRtType(clang::QualType type);
        static bool IsCppWinRtPrimitive(std::string const& type);
        bool IsRuntimeClassMethodType(clang::QualType type, bool projectedOnly = false);
        bool IsEventRevoker(clang::CXXMethodDecl* method);
        bool IsEventRegistrar(clang::CXXMethodDecl* method);
        bool IsConstructor(clang::CXXMethodDecl* method);
        bool IsDestructor(clang::CXXMethodDecl* method);
        std::optional<MethodKind> GetRuntimeClassMethodKind(clang::CXXMethodDecl* method);
        static clang::CXXRecordDecl* StripReferenceAndGetClassDecl(clang::QualType type);
        std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::QualType type);
        std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::CXXRecordDecl* record, bool implementationOnly = false);
        static std::vector<std::string> GetWinRtNamespaces(clang::NamedDecl* decl);
        std::optional<std::string> GetLocFilePath(clang::CXXRecordDecl* record);
        std::string GetLocFileName(clang::CXXRecordDecl* record);
    };
}