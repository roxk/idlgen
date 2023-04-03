#include "clang/AST/ASTContext.h"
#include "clang/Frontend/CompilerInstance.h"
#include "RuntimeClassVisitor.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Lex/Preprocessor.h"
#include <iostream>
#include <string>
#include <cassert>

idlgen::RuntimeClassVisitor::RuntimeClassVisitor(clang::CompilerInstance& ci, llvm::raw_ostream& out) :
    astContext(ci.getASTContext()),
    out(std::move(out))
{
}

void idlgen::RuntimeClassVisitor::Reset()
{
    implementationTypes.clear();
}

std::unordered_map<std::string, std::string> idlgen::RuntimeClassVisitor::cxxTypeToWinRtTypeMap{ idlgen::RuntimeClassVisitor::initCxxTypeToWinRtTypeMap() };

bool idlgen::RuntimeClassVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* record)
{
    auto location{ record->getLocation() };
    auto isMain{ astContext.getSourceManager().isInMainFile(location) };
    if (!isMain)
    {
        // We skip checking whether the file contains impl class for built-in headers.
        // TODO: Outright skip parsing them?
        auto fileNameOpt{ GetLocFilePath(record) };
        if (!fileNameOpt) { return true; }
        auto& fileName{ *fileNameOpt };
        if (fileName.find("winrt/") != std::string::npos || 
            fileName.find("Microsoft Visual Studio") != std::string::npos || 
            fileName.find("Windows Kits") != std::string::npos)
        {
            return true;
        }
        auto kindOpt = GetRuntimeClassKind(record, true);
        if (!kindOpt) { return true; }
        if (auto kind = *kindOpt; kind == idlgen::RuntimeClassKind::Implementation)
        {
            implementationTypes.insert({ record->getNameAsString(), record });
        }
        return true;
    }
    std::cout << "is main file" << std::endl;
    if (!GetRuntimeClassKind(record)) { return true; }
    std::cout << "is runtime class" << std::endl;
    std::vector<std::string> namespaces{ GetWinRtNamespaces(record) };
    if (!namespaces.empty() && namespaces.back() == "factory_implementation") { return true; }
    std::set<std::string> includes;
    std::vector<IdlGenAttr> attrs;
    std::optional<std::string> extend;
    std::set<clang::CXXMethodDecl*> ctors;
    std::map<std::string, MethodGroup> methodGroups;
    std::set<clang::CXXMethodDecl*> events;
    auto methods(record->methods());
    auto thisClassFileName = GetLocFileName(record);
    for (auto&& method : methods)
    {
        auto methodKind{ GetRuntimeClassMethodKind(method) };
        if (!methodKind) { continue; }
        if (IsDestructor(method)) { continue; }
        if (IsConstructor(method))
        {
            ctors.insert(method);
            continue;
        }
        if (IsEventRevoker(method)) { continue; }
        if (IsEventRegistrar(method))
        {
            events.insert(method);
            continue;
        }
        auto& group = GetMethodGroup(methodGroups, method->getNameAsString());
        auto findIncludeForParams = [&]()
        {
            auto params{ method->parameters() };
            for (auto&& param : params)
            {
                FindFileToInclude(includes, thisClassFileName, param->getType());
            }
        };
        if (methodKind == idlgen::MethodKind::Setter)
        {
            group.setter = method;
            findIncludeForParams();
        }
        else if (methodKind == idlgen::MethodKind::Getter)
        {
            group.getter = method;
            FindFileToInclude(includes, thisClassFileName, method->getReturnType());
        }
        else
        {
            group.method = method;
            FindFileToInclude(includes, thisClassFileName, method->getReturnType());
            findIncludeForParams();
        }
    }
    auto cxxAttrs{ record->attrs() };
    for (auto&& attr : cxxAttrs)
    {
        auto idlGenAttr = GetIdlGenAttr(attr);
        if (!idlGenAttr) { continue; }
        assert(!idlGenAttr->args.empty());
        if (idlGenAttr->type == IdlGenAttrType::Attribute) { attrs.emplace_back(std::move(*idlGenAttr)); }
        else if (idlGenAttr->type == IdlGenAttrType::Extend) { extend = idlGenAttr->args[0]; }
        else if (idlGenAttr->type == IdlGenAttrType::Import)
        {
            auto imports{ idlGenAttr->args };
            for (auto&& importFile : imports)
            {
                includes.insert(importFile);
            }
        }
    }
    // Generate idl
    for (auto&& include : includes)
    {
        out << "import \"" << include << "\";" << "\n";
    }
    out << "namespace ";
    const auto namespaceCount = namespaces.size();
    for (size_t i = 0; i < namespaceCount; ++i)
    {
        out << namespaces[i];
        if (i + 1 < namespaceCount) { out << "."; }
    }
    out << "\n";
    out << "{" << "\n";
    for (auto&& attr : attrs)
    {
        if (attr.type != IdlGenAttrType::Attribute) { continue; }
        auto& args{ attr.args };
        for (auto&& arg : args)
        {
            out << "[" << arg << "]" << "\n";
        }
    }
    out << "runtimeclass " << record->getNameAsString();
    if (extend)
    {
        out << " : " << *extend;
    }
    out << "\n";
    out << "{" << "\n";
    auto printMethodParams = [&](clang::CXXMethodDecl* method)
    {
        out << "(";
        auto params{ method->parameters() };
        const auto paramCount = params.size();
        for (size_t i = 0; i < paramCount; ++i)
        {
            auto param{ params[i] };
            out << TranslateCxxTypeToWinRtType(param->getType()) << " " << param->getNameAsString();
            if (i + 1 < paramCount) { out << ", "; }
        }
        out << ");";
    };
    for (auto&& ctor : ctors)
    {
        out << ctor->getNameAsString();
        printMethodParams(ctor);
        out << "\n";
    }
    for (auto&& entry : methodGroups)
    {
        auto& group{ entry.second };
        if (group.getterOrElse()->isStatic() && group.setterOrElse()->isStatic())
        {
            out << "static ";
        }
        auto returnType{ TranslateCxxTypeToWinRtType(group.getterOrElse()->getReturnType())};
        out << returnType << " " << entry.first;
        if (group.IsGetter())
        {
            out << "{get;};";
        }
        else if (group.IsProperty())
        {
            out << ";";
        }
        else
        {
            printMethodParams(group.setterOrElse());
        }
        out << "\n";
    }
    for (auto&& ev : events)
    {
        assert(ev->parameters().size() > 0);
        auto handler{ TranslateCxxTypeToWinRtType(ev->parameters().front()->getType()) };
        out << "event " << handler << " " << ev->getNameAsString() << ";" << "\n";
    }
    out << "}" << "\n";
    out << "}" << "\n";
    return true;
}

std::optional<idlgen::IdlGenAttr> idlgen::RuntimeClassVisitor::GetIdlGenAttr(clang::Attr* attr)
{
    if (attr->getScopeName()->getName() != "clang" || attr->getAttrName()->getName() != "annotate")
    {
        return std::nullopt;
    }
    auto annotationGetter = [&]() -> std::optional<std::string>
    {
        std::string pretty;
        llvm::raw_string_ostream prettyStream(pretty);
        attr->printPretty(prettyStream, clang::PrintingPolicy(clang::LangOptions()));
        llvm::SmallVector<llvm::StringRef> splitted;
        llvm::SplitString(pretty, splitted, "\"");
        constexpr auto rawAttrContentIndex = 1;
        constexpr auto expectedRawAttrSize = 3;
        if (splitted.size() != expectedRawAttrSize) { return std::nullopt; }
        return splitted[rawAttrContentIndex].str();
    };
    auto annotationOpt{ annotationGetter() };
    if (!annotationOpt) { std::nullopt; }
    auto& annotation{ *annotationOpt };
    if (annotation.find("idlgen::") != 0) { return std::nullopt; }
    auto equalIndex = annotation.find("=");
    if (equalIndex == std::string::npos) { return std::nullopt; }
    auto idlGenAttr{ std::string_view(annotation) };
    constexpr auto idlgenAttrHeaderCount = 8;
    idlGenAttr = idlGenAttr.substr(idlgenAttrHeaderCount, equalIndex - idlgenAttrHeaderCount);
    auto args{ annotation.substr(equalIndex + 1) };
    if (idlGenAttr == "attribute")
    {
        if (args.empty()) { return std::nullopt; }
        return IdlGenAttr{ IdlGenAttrType::Attribute, {std::move(args)}};
    }
    else if (idlGenAttr == "extend")
    {
        if (args.empty()) { return std::nullopt; }
        return IdlGenAttr{ IdlGenAttrType::Extend, {std::move(args)}};
    }
    else if (idlGenAttr == "import")
    {
        llvm::SmallVector<llvm::StringRef> splitted;
        llvm::SplitString(args, splitted, ",");
        if (splitted.empty()) { std::nullopt; }
        std::vector<std::string> result;
        result.reserve(splitted.size());
        for (auto&& importFile : splitted)
        {
            result.emplace_back(importFile.str());
        }
        return IdlGenAttr{ IdlGenAttrType::Import, result };
    }
    return std::nullopt;
}

idlgen::MethodGroup& idlgen::RuntimeClassVisitor::GetMethodGroup(std::map<std::string, MethodGroup>& methodGroups, std::string const& methodName)
{
    if (auto result{ methodGroups.find(methodName) }; result != methodGroups.end())
    {
        return result->second;
    }
    auto entry{ methodGroups.insert({ methodName, MethodGroup{nullptr, nullptr} }) };
    return entry.first->second;
}

void idlgen::RuntimeClassVisitor::FindFileToInclude(std::set<std::string>& includes, std::string const& thisClassFileName, clang::QualType type)
{
    auto paramRecord{ StripReferenceAndGetClassDecl(type) };
    if (paramRecord == nullptr) { return; }
    if (GetRuntimeClassKind(paramRecord))
    {
        auto implType = implementationTypes.find(paramRecord->getNameAsString());
        if (implType == implementationTypes.end()) { return; }
        auto paramClassFileName{ GetLocFileName(implType->second) };
        if (thisClassFileName != paramClassFileName)
        {
            while (true)
            {
                auto extension{ llvm::sys::path::extension(paramClassFileName) };
                if (extension.empty()) { break; }
                if (auto extensionIndex = paramClassFileName.rfind(extension); extensionIndex != std::string::npos)
                {
                    paramClassFileName.erase(extensionIndex);
                    continue;
                }
                break;
            }
            includes.insert(paramClassFileName + ".idl");
        }
    }
}

std::unordered_map<std::string, std::string> idlgen::RuntimeClassVisitor::initCxxTypeToWinRtTypeMap()
{
    // See https://learn.microsoft.com/en-us/uwp/midl-3/intro#types
    return
    {
        {"void", "void"},
        {"winrt::hstring", "String"},
        {"winrt::guid", "Guid"},
        {"winrt::Windows::Foundation::EventHandler", "Windows.Foundation.EventHandler"},
        {"winrt::Windows::Foundation::IInspectable", "Object"},
        {"winrt::Windows::Foundation::TimeSpan", "Windows.Foundation.TimeSpan"},
        {"winrt::Windows::Foundation::DateTime", "Windows.Foundation.DateTime"},
        {"bool", "Boolean"},
        // TODO: Should we check sizeof(int)?
        {"short", "Int16"},
        {"int", "Int32"},
        {"long", "Int32"},
        {"long long", "Int64"},
        {"unsigned char", "UInt8"},
        {"unsigned short", "UInt16"},
        {"unsigned int", "UInt32"},
        {"unsigned long", "UInt32"},
        {"unsigned long long", "UInt64"},
        {"char16_t", "Char16"},
        {"float", "Single"},
        {"double", "Double"},
    };
}

std::string idlgen::RuntimeClassVisitor::TranslateCxxTypeToWinRtType(clang::QualType type)
{
    if (type->isVoidType()) { return "void"; }
    else if (type->isBooleanType()) { return "Boolean"; }
    std::string qualifiedName;
    clang::NamedDecl* decl{ nullptr };
    auto record{ StripReferenceAndGetClassDecl(type) };
    if (record != nullptr)
    {
        llvm::raw_string_ostream typeOs{ qualifiedName };
        record->printQualifiedName(typeOs);
        decl = record;
    }
    else if (type->isScalarType())
    {
        if (type->isEnumeralType())
        {
            auto enumType{ clang::cast<clang::EnumType>(type.getCanonicalType()) };
            auto enumDecl{ enumType->getDecl() };
            llvm::raw_string_ostream typeOs{ qualifiedName };
            enumDecl->printQualifiedName(typeOs);
            decl = enumDecl;
        }
        else
        {
            qualifiedName = type.getNonReferenceType().getCanonicalType().getAsString();
        }
    }
    else { return "error-type"; }
    if (auto result{ cxxTypeToWinRtTypeMap.find(qualifiedName) }; result != cxxTypeToWinRtTypeMap.end())
    {
        return result->second;
    }
    if (decl == nullptr) { return "error-type"; }
    auto name{ decl->getNameAsString() };
    const auto namespaces{ GetWinRtNamespaces(decl) };
    std::string qualifiedWinRtName;
    qualifiedWinRtName.reserve(name.size() + 20);
    const auto namespaceCount = namespaces.size();
    for (auto&& namespaceStr : namespaces)
    {
        qualifiedWinRtName += namespaceStr;
        qualifiedWinRtName += ".";
    }
    qualifiedWinRtName += name;
    return qualifiedWinRtName;
}

bool idlgen::RuntimeClassVisitor::IsCppWinRtPrimitive(std::string const& type)
{
    return type == "winrt::hstring" || type == "winrt::event_token"
        || type == "winrt::Windows::Foundation::IInspectable";
}

bool idlgen::RuntimeClassVisitor::IsRuntimeClassMethodType(clang::QualType type, bool projectedOnly)
{
    if (type->isVoidType()) { return true; }
    if (type->isScalarType())
    {
        auto scalarType{ type->getScalarTypeKind() };
        return scalarType == clang::Type::STK_Integral
            || scalarType == clang::Type::STK_Bool
            || scalarType == clang::Type::STK_Floating;
    }
    auto record{ StripReferenceAndGetClassDecl(type) };
    if (record == nullptr)
    {
        auto rawType{ type.getNonReferenceType().getUnqualifiedType().getAsString() };
        out << rawType
            << " struct=" << type->isStructureType()
            << " class=" << type->isClassType()
            << " incomplete=" << type->isIncompleteType()
            << " reference=" << type->isReferenceType()
            << "\n";
        return false;
    }
    if (!record->isCompleteDefinition()) { return false; }
    if (record->isPOD()) { return true; }
    std::string qualifiedName;
    llvm::raw_string_ostream typeOs{ qualifiedName };
    record->printQualifiedName(typeOs);
    if (IsCppWinRtPrimitive(qualifiedName)) { return true; }
    if (auto kindOpt = GetRuntimeClassKind(record); 
        kindOpt && (!projectedOnly || *kindOpt == idlgen::RuntimeClassKind::Projected))
    {
        return true;
    }
    return false;
}

bool idlgen::RuntimeClassVisitor::IsEventRevoker(clang::CXXMethodDecl* method)
{
    if (!GetRuntimeClassMethodKind(method)) return false;
    auto params{ method->parameters() };
    for (auto&& param : params)
    {
        if (param->getType().getNonReferenceType().getUnqualifiedType().getAsString() == "winrt::event_token")
        {
            return true;
        }
    }
    return false;
}

bool idlgen::RuntimeClassVisitor::IsEventRegistrar(clang::CXXMethodDecl* method)
{
    if (!GetRuntimeClassMethodKind(method)) return false;
    return method->getReturnType().getNonReferenceType().getUnqualifiedType().getAsString() == "winrt::event_token";
}

bool idlgen::RuntimeClassVisitor::IsConstructor(clang::CXXMethodDecl* method)
{
    return method->getDeclKind() == clang::Decl::Kind::CXXConstructor;
}

bool idlgen::RuntimeClassVisitor::IsDestructor(clang::CXXMethodDecl* method)
{
    return method->getDeclKind() == clang::Decl::Kind::CXXDestructor;
}

std::optional<idlgen::MethodKind> idlgen::RuntimeClassVisitor::GetRuntimeClassMethodKind(clang::CXXMethodDecl* method)
{
    if (method->getAccess() != clang::AccessSpecifier::AS_public) { return std::nullopt; }
    auto params{ method->parameters() };
    auto returnType{ method->getReturnType() };
    if (params.size() == 0)
    {
        if (returnType->isVoidType()) { return idlgen::MethodKind::Method; }
        return IsRuntimeClassMethodType(returnType) ? std::optional(idlgen::MethodKind::Getter) : std::nullopt;
    }
    if (params.size() == 1)
    {
        auto paramType{ params[0]->getType()};
        if (returnType->isVoidType())
        {
            return IsRuntimeClassMethodType(paramType, true) ? std::optional(idlgen::MethodKind::Setter) : std::nullopt;
        }
        else
        {
            return IsRuntimeClassMethodType(returnType) && IsRuntimeClassMethodType(paramType, true)
                ? std::optional(idlgen::MethodKind::Method)
                : std::nullopt;
        }
    }
    // Ordinary methods
    if (!IsRuntimeClassMethodType(returnType)) { return std::nullopt; }
    for (auto&& param : params)
    {
        if (!IsRuntimeClassMethodType(param->getType(), true)) { return std::nullopt; }
    }
    return idlgen::MethodKind::Method;
}

clang::CXXRecordDecl* idlgen::RuntimeClassVisitor::StripReferenceAndGetClassDecl(clang::QualType type)
{
    return type->isReferenceType() ? type.getNonReferenceType()->getAsCXXRecordDecl() : type->getAsCXXRecordDecl();
}

std::optional<idlgen::RuntimeClassKind> idlgen::RuntimeClassVisitor::GetRuntimeClassKind(clang::QualType type)
{
    auto record{ StripReferenceAndGetClassDecl(type) };
    if (record == nullptr) { return std::nullopt; }
    return GetRuntimeClassKind(record);
}

std::optional<idlgen::RuntimeClassKind> idlgen::RuntimeClassVisitor::GetRuntimeClassKind(clang::CXXRecordDecl* record, bool implementationOnly)
{
    auto className{ record->getNameAsString() };
    std::cout << "Checking if " << className << " is runtime class" << std::endl;
    auto filePathOpt{ GetLocFilePath(record) };
    if (implementationOnly)
    {
        auto parentContext{ record->getParent() };
        if (parentContext == nullptr) { return std::nullopt; }
        if (!parentContext->isNamespace()) { return std::nullopt; }
        auto namespaceDecl = static_cast<clang::NamespaceDecl*>(parentContext);
        if (namespaceDecl->getNameAsString() != "implementation") { return std::nullopt; }
    }
    if (!record->isCompleteDefinition())
    {
        std::cout << className << " is not complete" << std::endl;
        return std::nullopt;
    }
    auto bases{ record->bases() };
    for (auto&& base : bases)
    {
        std::cout << "Checking base " << base.getType().getAsString() << std::endl;
        auto baseType{ base.getType().getTypePtrOrNull() };
        if (baseType == nullptr) { continue; }
        auto cxxType{ baseType->getAsCXXRecordDecl() };
        if (cxxType == nullptr) { continue; }
        // TODO: Need to check full name as Windows::Foundation::IInspectable instead
        if (cxxType->getName() == "IInspectable")
        {
            return idlgen::RuntimeClassKind::Projected;
        }
        if (cxxType->getDeclKind() == clang::Decl::Kind::ClassTemplateSpecialization)
        {
            auto spec{ static_cast<clang::ClassTemplateSpecializationDecl*>(cxxType) };
            auto templateName{ spec->getNameAsString() };
            std::cout << templateName << " is a template specialization" << std::endl;
            auto params{ spec->getTemplateArgs().asArray() };
            for (auto&& param : params)
            {
                auto paramKind = param.getKind();
                if (paramKind != clang::TemplateArgument::ArgKind::Type)
                {
                    std::cout << "Tempalte param is not a type" << std::endl;
                    continue;
                }
                std::cout << "Checking param " << param.getAsType().getAsString() << std::endl;
                auto type{ param.getAsType()->getAsCXXRecordDecl() };
                if (type == nullptr) { continue; }
                auto templateParamTypeName{ type->getNameAsString() };
                std::cout << "templateParamTypeName=" << templateParamTypeName << " className=" << className << " templateName=" << templateName << std::endl;
                if (templateParamTypeName == std::string_view(templateName).substr(0, templateName.size() - 1) && 
                    templateParamTypeName == className)
                {
                    return idlgen::RuntimeClassKind::Implementation;
                }
            }
        }
        if (auto opt = GetRuntimeClassKind(cxxType, implementationOnly))
        {
            return *opt;
        }
    }
    return std::nullopt;
}

std::vector<std::string> idlgen::RuntimeClassVisitor::GetWinRtNamespaces(clang::NamedDecl* record)
{
    std::vector<std::string> namespaces;
    auto parentContext = record->getDeclContext();
    while (parentContext != nullptr)
    {
        if (!parentContext->isNamespace()) { break; }
        auto namespaceDecl = static_cast<clang::NamespaceDecl*>(parentContext);
        auto name{ namespaceDecl->getNameAsString() };
        if (!(name == "winrt" || name == "implementation")) { namespaces.emplace_back(std::move(name)); }
        parentContext = parentContext->getParent();
    }
    std::reverse(namespaces.begin(), namespaces.end());
    return namespaces;
}

std::optional<std::string> idlgen::RuntimeClassVisitor::GetLocFilePath(clang::CXXRecordDecl* record)
{
    auto& srcManager{ astContext.getSourceManager() };
    auto file{ srcManager.getFileEntryForID(srcManager.getFileID(record->getLocation())) };
    if (file == nullptr) { return std::nullopt; }
    return file->getName().data();
}

std::string idlgen::RuntimeClassVisitor::GetLocFileName(clang::CXXRecordDecl* record)
{
    auto& srcManager{ astContext.getSourceManager() };
    auto file{ srcManager.getFileEntryForID(srcManager.getFileID(record->getLocation())) };
    return llvm::sys::path::filename(file->getName()).str();
}
