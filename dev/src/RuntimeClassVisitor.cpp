#include "clang/AST/ASTContext.h"
#include "clang/Sema/Template.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Parse/RAIIObjectsForParser.h"
#include "RuntimeClassVisitor.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Lex/Preprocessor.h"
#include <iostream>
#include <string>
#include <cassert>

idlgen::RuntimeClassVisitor::RuntimeClassVisitor(clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose) :
    ci(ci),
    astContext(ci.getASTContext()),
    out(std::move(out)),
    verbose(verbose)
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
    debugPrint([]() { std::cout << "is main file" << std::endl; });
    if (!GetRuntimeClassKind(record)) { return true; }
    debugPrint([]() { std::cout << "is runtime class" << std::endl; });
    std::vector<std::string> namespaces{ GetWinRtNamespaces(record) };
    if (!namespaces.empty() && namespaces.back() == "factory_implementation") { return true; }
    std::set<std::string> includes;
    std::vector<IdlGenAttr> attrs;
    std::optional<std::string> extend;
    std::set<clang::CXXMethodDecl*> ctors;
    std::map<std::string, MethodGroup> methodGroups;
    std::set<clang::CXXMethodDecl*> events;
    auto cxxAttrs{ record->attrs() };
    for (auto&& attr : cxxAttrs)
    {
        auto idlGenAttr = GetIdlGenAttr(attr);
        if (!idlGenAttr) { continue; }
        if (idlGenAttr->type == IdlGenAttrType::Hide) { return true; }
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
    auto methods(record->methods());
    auto thisClassFileName = GetLocFileName(record);
    for (auto&& method : methods)
    {
        debugPrint([&]()
            {
                std::cout << "Checking if " << method->getNameAsString() << " is runtime class method" << std::endl;
            });
        auto methodKind{ GetRuntimeClassMethodKind(method) };
        if (!methodKind) { continue; }
        auto params{ method->parameters() };
        for (auto&& param : params)
        {
            FindFileToInclude(includes, thisClassFileName, param->getType());
        }
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
        auto& group = GetMethodGroup(methodGroups, method);
        if (methodKind == idlgen::MethodKind::Setter)
        {
            group.setter = method;
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
        out << returnType << " " << group.methodName;
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
    if (equalIndex == std::string::npos) { equalIndex = annotation.size(); }
    auto idlGenAttr{ std::string_view(annotation) };
    constexpr auto idlgenAttrHeaderCount = 8;
    idlGenAttr = idlGenAttr.substr(idlgenAttrHeaderCount, equalIndex - idlgenAttrHeaderCount);
    std::string args{ equalIndex >= annotation.size() ? "" : annotation.substr(equalIndex + 1)};
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
    else if (idlGenAttr == "hide")
    {
        return IdlGenAttr{ IdlGenAttrType::Hide, {} };
    }
    return std::nullopt;
}

idlgen::MethodGroup& idlgen::RuntimeClassVisitor::GetMethodGroup(std::map<std::string, MethodGroup>& methodGroups, clang::CXXMethodDecl* method)
{
    auto methodName{ method->getNameAsString() };
    auto tryPrintParamName = [&](std::string& name, clang::QualType type)
    {
        if (!type->isVoidType())
        {
            name += "_";
            name += TranslateCxxTypeToWinRtType(type);
        }
    };
    auto paramStrGetter = [&]()
    {
        std::string name;
        auto returnType{ method->getReturnType() };
        tryPrintParamName(name, returnType);
        auto params{ method->parameters() };
        for (auto&& param : params)
        {
            tryPrintParamName(name, param->getType());
        }
        return name;
    };
    auto key{ methodName + paramStrGetter() };
    if (auto result{ methodGroups.find(key) }; result != methodGroups.end())
    {
        return result->second;
    }
    auto entry{ methodGroups.insert({ std::move(key), MethodGroup{ std::move(methodName), nullptr, nullptr } }) };
    return entry.first->second;
}

void idlgen::RuntimeClassVisitor::FindFileToInclude(std::set<std::string>& includes, std::string const& thisClassFileName, clang::QualType type)
{
    auto paramRecord{ StripReferenceAndGetClassDecl(type) };
    if (paramRecord == nullptr) { return; }
    debugPrint([&]() { std::cout << "Looking for include for " << paramRecord->getNameAsString() << std::endl; });
    if (IsRuntimeClassMethodType(type))
    {
        if (auto templateSpec = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(paramRecord))
        {
            debugPrint([&]() { std::cout << "Looking also for include for template params" << std::endl; });
            auto params{ templateSpec->getTemplateArgs().asArray() };
            for (auto&& param : params)
            {
                if (param.getKind() != clang::TemplateArgument::ArgKind::Type) { continue; }
                FindFileToInclude(includes, thisClassFileName, param.getAsType());
            }
        }
        auto implType = implementationTypes.find(paramRecord->getNameAsString());
        if (implType == implementationTypes.end()) { return; }
        auto paramClassFileName{ GetLocFileName(implType->second) };
        if (thisClassFileName != paramClassFileName)
        {
            auto extension{ llvm::sys::path::extension(paramClassFileName) };
            if (auto extensionIndex = paramClassFileName.rfind(extension); extensionIndex != std::string::npos)
            {
                paramClassFileName.erase(extensionIndex);
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
    // TODO: Handle template
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
    if (auto templateSpec = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(decl))
    {
        auto params{ templateSpec->getTemplateArgs().asArray() };
        std::string paramString;
        for (auto&& param : params)
        {
            if (param.getKind() != clang::TemplateArgument::ArgKind::Type) { continue; }
            auto type{ param.getAsType() };
            paramString += TranslateCxxTypeToWinRtType(type);
            paramString += ", ";
        }
        if (!paramString.empty())
        {
            const auto expectedLastCommaSpaceIndex = paramString.size() - 2;
            const auto lastCommaSpaceIndex = paramString.rfind(", ");
            if (lastCommaSpaceIndex == expectedLastCommaSpaceIndex) { paramString.erase(expectedLastCommaSpaceIndex); }
            qualifiedWinRtName += "<";
            qualifiedWinRtName += paramString;
            qualifiedWinRtName += ">";
        }
    }
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
        debugPrint([&]()
            {
                auto rawType{ type.getNonReferenceType().getUnqualifiedType().getAsString() };
                std::cout << rawType
                    << " struct=" << type->isStructureType()
                    << " class=" << type->isClassType()
                    << " incomplete=" << type->isIncompleteType()
                    << " reference=" << type->isReferenceType()
                    << "\n";
            });
        return false;
    }
    if (!record->isCompleteDefinition())
    {
        debugPrint([&]()
            {
                std::cout << record->getNameAsString() << " is not complete" << std::endl;
            });
        // We have to count incomplete type as OK since clang's AST wouldn't make template reference
        // (e.g. TypedHandler<T, A> const&) a complete type but we definitely want to support these
        // class...
        return true;
    }
    if (record->isPOD()) { return true; }
    std::string qualifiedName;
    llvm::raw_string_ostream typeOs{ qualifiedName };
    record->printQualifiedName(typeOs);
    if (IsCppWinRtPrimitive(qualifiedName)) { return true; }
    auto kindOpt = GetRuntimeClassKind(record);
    if (kindOpt && (!projectedOnly || *kindOpt == idlgen::RuntimeClassKind::Projected))
    {
        return true;
    }
    return false;
}

bool idlgen::RuntimeClassVisitor::IsEventRevoker(clang::CXXMethodDecl* method)
{
    auto params{ method->parameters() };
    if (params.size() != 1) { return false; }
    auto record = params.front()->getType()->getAsCXXRecordDecl();
    if (record == nullptr) { return false; }
    return GetQualifiedName(record) == "winrt::event_token";
}

bool idlgen::RuntimeClassVisitor::IsEventRegistrar(clang::CXXMethodDecl* method)
{
    auto returnRecord = method->getReturnType()->getAsCXXRecordDecl();
    if (returnRecord == nullptr) { return false; }
    return GetQualifiedName(returnRecord) == "winrt::event_token";
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
    debugPrint([&]() {std::cout << "Checking if " << className << " is runtime class" << std::endl; });
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
        debugPrint([&]() {std::cout << className << " is not complete" << std::endl; });
        return std::nullopt;
    }
    auto bases{ record->bases() };
    for (auto&& base : bases)
    {
        debugPrint([&]() { std::cout << "Checking base " << base.getType().getAsString() << std::endl; });
        auto baseType{ base.getType().getTypePtrOrNull() };
        if (baseType == nullptr) { continue; }
        auto cxxType{ baseType->getAsCXXRecordDecl() };
        if (cxxType == nullptr) { continue; }
        // TODO: Need to check full name as Windows::Foundation::IInspectable instead
        if (cxxType->getName() == "IInspectable" || cxxType->getName() == "IUnknown")
        {
            return idlgen::RuntimeClassKind::Projected;
        }
        auto templateSpecType{ baseType->getAs<clang::TemplateSpecializationType>() };
        auto spec{ clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(cxxType) };
        if (templateSpecType != nullptr && spec != nullptr)
        {
            auto templateDecl{ templateSpecType->getTemplateName().getAsTemplateDecl() };
            auto templateName{ templateDecl->getNameAsString() };
            debugPrint([&]() { std::cout << templateName << " is a template specialization" << std::endl; });
            auto params{ spec->getTemplateArgs().asArray() };
            for (auto&& param : params)
            {
                auto paramKind = param.getKind();
                if (paramKind != clang::TemplateArgument::ArgKind::Type)
                {
                    debugPrint([&]()
                        {
                            std::cout << "Template param ";
                            param.print(clang::LangOptions(), llvm::outs(), true);
                            std::cout << " is not a type" << std::endl;
                        });
                    continue;
                }
                auto type{ param.getAsType()->getAsCXXRecordDecl() };
                if (type == nullptr)
                {
                    debugPrint([&]() { std::cout << param.getAsType().getAsString() << " is not a CXXRecord" << std::endl; });
                    continue;
                }
                auto templateParamTypeName{ type->getNameAsString() };
                auto expectedParamTypeName{ std::string_view(templateName).substr(0, templateName.size() - 1) };
                debugPrint([&]()
                    {
                        std::cout << "templateParamTypeName=" << templateParamTypeName
                            << " className=" << className
                            << " templateName=" << templateName
                            << " expectedParamTypeName=" << expectedParamTypeName
                            << " cxxTypeName=" << cxxType->getName().data()
                            << std::endl;
                    });
                if (templateParamTypeName == expectedParamTypeName &&
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

std::string idlgen::RuntimeClassVisitor::GetQualifiedName(clang::CXXRecordDecl* record)
{
    std::string qualifiedName;
    llvm::raw_string_ostream typeOs{ qualifiedName };
    record->printQualifiedName(typeOs);
    return qualifiedName;
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
