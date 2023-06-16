#pragma once

#include "clang/AST/RecursiveASTVisitor.h"
#include "llvm/ADT/StringRef.h"
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>
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
    Hide,
    Property,
    Method,
    Overridable,
    Protected,
    Sealed
};
struct IdlGenAttr
{
    IdlGenAttrType type;
    std::vector<std::string> args;
};
class IdlgenVisitor;
class Printer
{
  public:
    virtual ~Printer()
    {
    }
    virtual void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) = 0;
};
class MethodGroup : public Printer
{
  private:
    std::string methodName;
    bool isStatic;
    bool isProtected;
    bool isVirtual;

  public:
    MethodGroup(
        std::string methodName,
        clang::CXXMethodDecl* method,
        clang::CXXMethodDecl* getter,
        clang::CXXMethodDecl* setter,
        bool isStatic,
        bool isProtected,
        bool isVirtual
    );
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
    bool IsMethod() const
    {
        return method != nullptr;
    }
    clang::CXXMethodDecl* GetterOrElse()
    {
        return method != nullptr ? method : getter != nullptr ? getter : setter;
    }
    clang::CXXMethodDecl* SetterOrElse()
    {
        return method != nullptr ? method : setter != nullptr ? setter : getter;
    }
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};
enum class PropertyKind
{
    Getter,
    Property
};
class PropertyMethodPrinter : public Printer
{
  private:
    std::string methodName;
    clang::QualType type;
    PropertyKind kind;
    bool isStatic;
    bool isProtected;
    bool isVirtual;

  public:
    PropertyMethodPrinter(
        std::string methodName, clang::QualType type, PropertyKind kind, bool isStatic, bool isProtected, bool isVirtual
    );
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};
struct MethodHolder
{
    std::unique_ptr<Printer> printer;
    std::optional<std::reference_wrapper<MethodGroup>> groupOpt;
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
enum class FieldKind
{
    PropertyDefault,
    MethodDefault
};
enum class ArrayKind
{
    In,
    Out,
    Ref
};
struct GetMethodResponse
{
    std::map<std::string, MethodHolder> holders;
    std::map<std::string, clang::CXXMethodDecl*> events;
    std::set<clang::CXXMethodDecl*> ctors;
};
class DelegatePrinter : public Printer
{
  private:
    clang::CXXRecordDecl* record;
    std::vector<clang::QualType> types;

  public:
    DelegatePrinter(clang::CXXRecordDecl* record, std::vector<clang::QualType> types);
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};
class StructPrinter : public Printer
{
  private:
    clang::CXXRecordDecl* record;
    std::vector<clang::FieldDecl*> fields;

  public:
    StructPrinter(clang::CXXRecordDecl* record, std::vector<clang::FieldDecl*> fields);
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};
class ClassPrinter : public Printer
{
  private:
    clang::CXXRecordDecl* record;
    GetMethodResponse response;
    std::optional<std::vector<clang::QualType>> extend;
    bool isSealed;

  public:
    ClassPrinter(
        clang::CXXRecordDecl* record,
        GetMethodResponse response,
        std::optional<std::vector<clang::QualType>> extend,
        bool isSealed
    );
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};
class InterfacePrinter : public Printer
{
  private:
    clang::CXXRecordDecl* record;
    GetMethodResponse response;
    std::optional<std::vector<clang::QualType>> extend;

  public:
    InterfacePrinter(
        clang::CXXRecordDecl* record, GetMethodResponse response, std::optional<std::vector<clang::QualType>> extend
    );
    void Print(IdlgenVisitor& visitor, llvm::raw_ostream& out) override;
};

class IdlgenVisitor : public clang::RecursiveASTVisitor<IdlgenVisitor>
{
  private:
    clang::CompilerInstance& ci;
    clang::ASTContext& astContext;
    llvm::raw_ostream& out;
    static std::unordered_map<std::string, std::string> cxxTypeToWinRtTypeMap;
    // Import source types = implementation types or authored types
    // Key uses WinRT type names, i.e. name as shown in idl. Therefore, prefix
    // _ would be trimmed
    std::unordered_map<std::string, clang::NamedDecl*> importSourceTypes;
    std::set<std::string> includes;
    std::unordered_set<std::string> getterTemplates;
    std::unordered_set<std::string> propertyTemplates;
    bool verbose;

  public:
    explicit IdlgenVisitor(
        clang::CompilerInstance& ci,
        llvm::raw_ostream& out,
        bool verbose,
        std::vector<std::string> const& getterTemplates,
        std::vector<std::string> const& propertyTemplates
    );

    void Reset();

    bool VisitCXXRecordDecl(clang::CXXRecordDecl* record);

    bool VisitEnumDecl(clang::EnumDecl* decl);

    void PrintNameSpaces(std::vector<std::string> namespaces);

    void PrintMethodParams(clang::CXXMethodDecl* method);

    void PrintEvent(std::string_view name, clang::CXXMethodDecl* method);

    std::string TranslateCxxTypeToWinRtType(clang::QualType type);

    template <typename Func> void debugPrint(Func&& func)
    {
        if (verbose)
        {
            func();
        }
    }

  private:
    std::optional<IdlGenAttr> GetIdlGenAttr(clang::Attr* attr);
    MethodGroup& GetOrCreateMethodGroup(
        std::map<std::string, MethodHolder>& methodHolders,
        clang::CXXMethodDecl* method,
        idlgen::MethodKind kind,
        std::string methodName,
        bool isStatic,
        bool isProtected,
        bool isVirtual
    );
    std::unique_ptr<idlgen::Printer> GetMethodPrinter(
        clang::NamedDecl* field, clang::QualType type, bool isStatic, bool isProtected, bool isVirtual
    );
    void FindFileToInclude(clang::QualType type);
    static std::unordered_map<std::string, std::string> initCxxTypeToWinRtTypeMap();
    GetMethodResponse GetMethods(clang::CXXRecordDecl* record, bool isPropertyDefault);
    static bool IsCppWinRtPrimitive(std::string const& type);
    bool IsRuntimeClassMethodType(clang::QualType type, bool projectedOnly = false);
    bool IsEventRevoker(clang::CXXMethodDecl* method);
    bool IsEventRegistrar(clang::CXXMethodDecl* method);
    bool IsConstructor(clang::CXXMethodDecl* method);
    bool IsDestructor(clang::CXXMethodDecl* method);
    bool ShouldSkipGenerating(clang::NamedDecl* decl);
    bool HasAttribute(clang::Decl* decl, idlgen::IdlGenAttrType type);
    bool IsProtected(clang::Decl* decl);
    std::optional<MethodKind> GetRuntimeClassMethodKind(bool isPropertyDefault, clang::CXXMethodDecl* method);
    std::optional<FieldKind> GetRuntimeClassFieldKind(bool isPropertyDefault, clang::ValueDecl* value);
    std::optional<ArrayKind> GetArrayKind(clang::NamedDecl* record);
    std::optional<ArrayKind> GetArrayKind(clang::QualType type);
    static clang::QualType StripReference(clang::QualType type);
    static clang::CXXRecordDecl* StripReferenceAndGetClassDecl(clang::QualType type);
    static const clang::NamedDecl* StripReferenceAndGetNamedDecl(clang::QualType type);
    std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::QualType type);
    std::optional<EnumKind> GetEnumKind(clang::EnumDecl* decl);
    std::optional<RuntimeClassKind> GetRuntimeClassKind(clang::CXXRecordDecl* record, bool implementationOnly = false);
    std::optional<std::vector<clang::QualType>> GetExtend(clang::CXXRecordDecl* record);
    static std::vector<std::string> GetWinRtNamespaces(clang::NamedDecl* decl);
    static std::string GetQualifiedName(clang::CXXRecordDecl* record);
    clang::QualType GetFirstTemplateTypeParam(clang::ClassTemplateSpecializationDecl const* templateSpecDecl);
    std::vector<clang::QualType> GetTemplateTypeParam(clang::ClassTemplateSpecializationDecl const* templateSpecDecl);
    std::optional<std::string> GetLocFilePath(clang::NamedDecl* decl);
    std::optional<std::string> GetLocFileName(clang::CXXRecordDecl* record);
    template <typename Func> void ForThisAndBaseMethods(clang::CXXRecordDecl const* record, Func&& func)
    {
        auto fieldMethods{record->methods()};
        for (auto&& fieldMethod : fieldMethods)
        {
            func(fieldMethod);
        }
        auto bases{record->bases()};
        for (auto&& base : bases)
        {
            auto baseType{base.getType().getTypePtrOrNull()};
            if (baseType == nullptr)
            {
                continue;
            }
            auto cxxType{baseType->getAsCXXRecordDecl()};
            if (cxxType == nullptr)
            {
                continue;
            }
            ForThisAndBaseMethods(cxxType, std::forward<Func>(func));
        }
    }
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is runtime class</returns>
    std::unique_ptr<Printer> TryHandleAsClass(
        clang::CXXRecordDecl* decl, bool isPropertyDefault, std::vector<idlgen::IdlGenAttr>& attrs
    );
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is interface</returns>
    std::unique_ptr<Printer> TryHandleAsInterface(clang::CXXRecordDecl* decl, bool isPropertyDefault);
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is delegate</returns>
    std::unique_ptr<idlgen::DelegatePrinter> TryHandleAsDelegate(clang::CXXRecordDecl* decl);
    /// <summary>
    /// </summary>
    /// <param name="decl"></param>
    /// <returns>True if is struct</returns>
    std::unique_ptr<Printer> TryHandleAsStruct(clang::CXXRecordDecl* decl);
    bool IsSingleBaseOfType(clang::CXXRecordDecl* decl, std::string_view name);
    bool IsBaseOfType(clang::CXXRecordDecl* decl, std::string_view name);
};
} // namespace idlgen