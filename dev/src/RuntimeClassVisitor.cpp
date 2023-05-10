#include "RuntimeClassVisitor.h"
#include "clang/AST/ASTContext.h"
#include "clang/AST/DeclCXX.h"
#include "clang/Frontend/CompilerInstance.h"
#include <cassert>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

constexpr auto nameAuthorEnum = "idlgen::author_enum";
constexpr auto nameAuthorEnumFlags = "idlgen::author_enum_flags";
constexpr auto nameAuthorStruct = "idlgen::author_struct";
constexpr auto nameAuthorDelegate = "idlgen::author_delegate";
constexpr auto nameAuthorInterface = "idlgen::author_interface";

idlgen::RuntimeClassVisitor::RuntimeClassVisitor(
    clang::CompilerInstance& ci,
    llvm::raw_ostream& out,
    bool verbose,
    std::vector<std::string> const& getterTemplates,
    std::vector<std::string> const& propertyTemplates
)
    : ci(ci), astContext(ci.getASTContext()), out(std::move(out)), verbose(verbose)
{
    for (auto&& getterTemplate : getterTemplates)
    {
        this->getterTemplates.insert(getterTemplate);
    }
    for (auto&& propertyTemplate : propertyTemplates)
    {
        this->propertyTemplates.insert(propertyTemplate);
    }
}

void idlgen::RuntimeClassVisitor::Reset()
{
    importSourceTypes.clear();
    includes.clear();
}

std::unordered_map<std::string, std::string> idlgen::RuntimeClassVisitor::cxxTypeToWinRtTypeMap{
    idlgen::RuntimeClassVisitor::initCxxTypeToWinRtTypeMap(),
};

bool idlgen::RuntimeClassVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* record)
{
    auto location{record->getLocation()};
    auto isMain{astContext.getSourceManager().isInMainFile(location)};
    if (!isMain)
    {
        if (ShouldSkipGenerating(record))
        {
            return true;
        }
        if (auto kindOpt = GetRuntimeClassKind(record, true);
            kindOpt && *kindOpt == idlgen::RuntimeClassKind::Implementation)
        {
            importSourceTypes.insert({record->getNameAsString(), record});
        }
        else if (IsSingleBaseOfType(record, nameAuthorStruct) || IsSingleBaseOfType(record, nameAuthorDelegate) || IsSingleBaseOfType(record, nameAuthorInterface))
        {
            importSourceTypes.insert({record->getNameAsString(), record});
        }
        return true;
    }
    debugPrint([]() { std::cout << "is main file" << std::endl; });
    std::vector<std::string> namespaces{GetWinRtNamespaces(record)};
    if (!namespaces.empty() && namespaces.back() == "factory_implementation")
    {
        return true;
    }
    std::vector<IdlGenAttr> attrs;
    auto cxxAttrs{record->attrs()};
    bool isPropertyDefault = false;
    for (auto&& attr : cxxAttrs)
    {
        auto idlGenAttr = GetIdlGenAttr(attr);
        if (!idlGenAttr)
        {
            continue;
        }
        if (idlGenAttr->type == IdlGenAttrType::Hide)
        {
            return true;
        }
        if (idlGenAttr->type == IdlGenAttrType::Attribute)
        {
            attrs.emplace_back(std::move(*idlGenAttr));
        }
        else if (idlGenAttr->type == IdlGenAttrType::Import)
        {
            auto imports{idlGenAttr->args};
            for (auto&& importFile : imports)
            {
                includes.insert(importFile);
            }
        }
        else if (idlGenAttr->type == IdlGenAttrType::Property)
        {
            isPropertyDefault = true;
        }
        else if (idlGenAttr->type == IdlGenAttrType::Method)
        {
            isPropertyDefault = false;
        }
    }
    std::unique_ptr<Printer> printer;
    if (printer = TryHandleAsInterface(record, isPropertyDefault))
    {
        debugPrint([]() { std::cout << "is interface" << std::endl; });
    }
    else if (printer = TryHandleAsDelegate(record))
    {
        debugPrint([]() { std::cout << "is delegate" << std::endl; });
    }
    else if (printer = TryHandleAsStruct(record))
    {
        debugPrint([]() { std::cout << "is struct" << std::endl; });
    }
    else if (printer = TryHandleAsClass(record, isPropertyDefault, attrs))
    {
        debugPrint([]() { std::cout << "is runtime class" << std::endl; });
    }
    else
    {
        return true;
    }
    // Generate idl
    for (auto&& include : includes)
    {
        out << "import \"" << include << "\";"
            << "\n";
    }
    PrintNameSpaces(namespaces);
    out << "\n";
    out << "{\n";
    for (auto&& attr : attrs)
    {
        if (attr.type != IdlGenAttrType::Attribute)
        {
            continue;
        }
        auto& args{attr.args};
        for (auto&& arg : args)
        {
            out << "[" << arg << "]"
                << "\n";
        }
    }
    if (printer != nullptr)
    {
        printer->Print(*this, out);
    }
    out << "}\n";
    return true;
}

bool idlgen::RuntimeClassVisitor::VisitEnumDecl(clang::EnumDecl* decl)
{
    auto location{decl->getLocation()};
    auto isMain{astContext.getSourceManager().isInMainFile(location)};
    auto kind{GetEnumKind(decl)};
    if (!isMain)
    {
        if (kind)
        {
            importSourceTypes.insert({decl->getNameAsString(), decl});
        }
        return true;
    }
    debugPrint([]() { std::cout << "is main file" << std::endl; });
    if (!kind)
    {
        return true;
    }
    debugPrint([&]() { std::cout << decl->getNameAsString() << " is WinRT enum" << std::endl; });
    std::vector<std::string> namespaces{GetWinRtNamespaces(decl)};
    PrintNameSpaces(namespaces);
    out << "\n";
    out << "{"
        << "\n";
    if (*kind == EnumKind::Flag)
    {
        out << "[flags]"
            << "\n";
    }
    out << "enum " << decl->getNameAsString() << "\n";
    out << "{"
        << "\n";
    auto values{decl->enumerators()};
    for (auto&& value : values)
    {
        auto radix{*kind == EnumKind::Normal ? 10 : 16};
        auto intValue = [&]()
        {
            if (*kind == EnumKind::Normal)
            {
                return std::to_string(value->getInitVal().getExtValue());
            }
            llvm::SmallVector<char> intValue;
            std::stringstream ss;
            ss << std::setfill('0') << std::setw(8) << value->getInitVal().getExtValue();
            return "0x" + ss.str();
        };
        out << value->getNameAsString() << " = " << intValue();
        out << ",\n";
    }
    out << "};"
        << "\n";
    out << "}"
        << "\n";
    return true;
}

std::optional<idlgen::IdlGenAttr> idlgen::RuntimeClassVisitor::GetIdlGenAttr(clang::Attr* attr)
{
    auto scopeName{attr->getScopeName()};
    if (scopeName == nullptr)
    {
        return std::nullopt;
    }
    auto attrName{attr->getAttrName()};
    if (attrName == nullptr)
    {
        return std::nullopt;
    }
    if (scopeName->getName() != "clang" || attrName->getName() != "annotate")
    {
        return std::nullopt;
    }
    auto annotationGetter = [&]() -> std::optional<std::string>
    {
        std::string pretty;
        llvm::raw_string_ostream prettyStream(pretty);
        attr->printPretty(prettyStream, clang::PrintingPolicy(clang::LangOptions()));
        llvm::SmallVector<llvm::StringRef> splitted;
        const auto firstQuoteIndex{pretty.find("\"")};
        const auto lastQuoteIndex{pretty.rfind("\"")};
        const std::string_view prettyView{pretty};
        if (firstQuoteIndex > 0)
        {
            splitted.emplace_back(prettyView.substr(0, firstQuoteIndex));
        }
        if (lastQuoteIndex >= firstQuoteIndex + 1)
        {
            const auto start{firstQuoteIndex + 1};
            splitted.emplace_back(prettyView.substr(start, lastQuoteIndex - start));
            if (lastQuoteIndex + 1 < prettyView.size())
            {
                splitted.emplace_back(prettyView.substr(lastQuoteIndex + 1));
            }
        }
        constexpr auto rawAttrContentIndex = 1;
        constexpr auto expectedRawAttrSize = 3;
        if (splitted.size() != expectedRawAttrSize)
        {
            return std::nullopt;
        }
        return splitted[rawAttrContentIndex].str();
    };
    auto annotationOpt{annotationGetter()};
    if (!annotationOpt)
    {
        return std::nullopt;
    }
    auto& annotation{*annotationOpt};
    if (annotation.find("idlgen::") != 0)
    {
        return std::nullopt;
    }
    auto equalIndex = annotation.find("=");
    if (equalIndex == std::string::npos)
    {
        equalIndex = annotation.size();
    }
    auto idlGenAttr{std::string_view(annotation)};
    constexpr auto idlgenAttrHeaderCount = 8;
    idlGenAttr = idlGenAttr.substr(idlgenAttrHeaderCount, equalIndex - idlgenAttrHeaderCount);
    std::string args{equalIndex >= annotation.size() ? "" : annotation.substr(equalIndex + 1)};
    if (idlGenAttr == "attribute")
    {
        if (args.empty())
        {
            return std::nullopt;
        }
        return IdlGenAttr{IdlGenAttrType::Attribute, {std::move(args)}};
    }
    else if (idlGenAttr == "extend")
    {
        if (args.empty())
        {
            return std::nullopt;
        }
        return IdlGenAttr{IdlGenAttrType::Extend, {std::move(args)}};
    }
    else if (idlGenAttr == "import")
    {
        llvm::SmallVector<llvm::StringRef> splitted;
        llvm::SplitString(args, splitted, ",");
        if (splitted.empty())
        {
            std::nullopt;
        }
        std::vector<std::string> result;
        result.reserve(splitted.size());
        for (auto&& importFile : splitted)
        {
            result.emplace_back(importFile.str());
        }
        return IdlGenAttr{IdlGenAttrType::Import, result};
    }
    else if (idlGenAttr == "hide")
    {
        return IdlGenAttr{IdlGenAttrType::Hide, {}};
    }
    else if (idlGenAttr == "property")
    {
        return IdlGenAttr{IdlGenAttrType::Property, {}};
    }
    else if (idlGenAttr == "method")
    {
        return IdlGenAttr{IdlGenAttrType::Method, {}};
    }
    return std::nullopt;
}

idlgen::MethodGroup& idlgen::RuntimeClassVisitor::GetOrCreateMethodGroup(
    std::map<std::string, MethodHolder>& methodGroups,
    clang::CXXMethodDecl* method,
    idlgen::MethodKind kind,
    std::string methodName,
    bool isStatic,
    bool isProtected
)
{
    auto tryPrintParamName = [&](std::string& name, clang::QualType type)
    {
        if (!type->isVoidType())
        {
            name += "_";
            name += TranslateCxxTypeToWinRtType(type);
        }
    };
    auto paramStrGetter = [&](std::string methodName)
    {
        std::string name;
        if (kind == idlgen::MethodKind::Method)
        {
            auto returnType{method->getReturnType()};
            tryPrintParamName(name, returnType);
            name += "_";
            name += std::move(methodName);
        }
        else
        {
            name = std::move(methodName);
            auto returnType{method->getReturnType()};
            tryPrintParamName(name, returnType);
        }
        auto params{method->parameters()};
        for (auto&& param : params)
        {
            tryPrintParamName(name, param->getType());
        }
        return name;
    };
    auto key{paramStrGetter(methodName)};
    if (auto result{methodGroups.find(key)}; result != methodGroups.end() && result->second.groupOpt.has_value())
    {
        return result->second.groupOpt.value();
    }
    auto group{std::make_unique<MethodGroup>(std::move(methodName), nullptr, nullptr, nullptr, isStatic, isProtected)};
    auto& groupRef{*group};
    auto entry{methodGroups.insert({std::move(key), MethodHolder{std::move(group), groupRef}})};
    auto& groupOpt = entry.first->second.groupOpt;
    assert(groupOpt.has_value());
    return *groupOpt;
}

std::unique_ptr<idlgen::Printer> idlgen::RuntimeClassVisitor::GetMethodPrinter(
    clang::NamedDecl* field, clang::QualType type, bool isStatic, bool isProtected
)
{
    auto record{type->getAsCXXRecordDecl()};
    if (record == nullptr)
    {
        return nullptr;
    }
    auto templateSpecDecl = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(record);
    if (templateSpecDecl == nullptr)
    {
        return nullptr;
    }
    auto templateSpecType{type->getAs<clang::TemplateSpecializationType>()};
    if (templateSpecType == nullptr)
    {
        return nullptr;
    }
    auto propertyType{GetFirstTemplateTypeParam(templateSpecDecl)};
    if (!propertyType.isNull())
    {
        auto templateName{templateSpecType->getTemplateName().getAsTemplateDecl()->getQualifiedNameAsString()};
        std::optional<std::unique_ptr<Printer>> printerOpt;
        if (getterTemplates.find(templateName) != getterTemplates.end())
        {
            return std::make_unique<PropertyMethodPrinter>(
                field->getNameAsString(), propertyType, PropertyKind::Getter, isStatic, isProtected
            );
        }
        else if (propertyTemplates.find(templateName) != propertyTemplates.end())
        {
            return std::make_unique<PropertyMethodPrinter>(
                field->getNameAsString(), propertyType, PropertyKind::Property, isStatic, isProtected
            );
        }
    }
    return nullptr;
}

void idlgen::RuntimeClassVisitor::FindFileToInclude(clang::QualType type)
{
    auto namedDecl{StripReferenceAndGetNamedDecl(type)};
    if (namedDecl == nullptr)
    {
        return;
    }
    debugPrint([&]() { std::cout << "Looking for include for " << namedDecl->getNameAsString() << std::endl; });
    if (IsRuntimeClassMethodType(type))
    {
        if (auto templateSpec = clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(namedDecl))
        {
            debugPrint([&]() { std::cout << "Looking also for include for template params" << std::endl; });
            auto params{templateSpec->getTemplateArgs().asArray()};
            for (auto&& param : params)
            {
                if (param.getKind() != clang::TemplateArgument::ArgKind::Type)
                {
                    continue;
                }
                FindFileToInclude(param.getAsType());
            }
        }
        auto implType = importSourceTypes.find(namedDecl->getNameAsString());
        if (implType == importSourceTypes.end())
        {
            return;
        }
        auto paramClassFilePathOpt{GetLocFilePath(implType->second)};
        if (!paramClassFilePathOpt)
        {
            debugPrint(
                [&]() {
                    std::cout << "Failed to get file path for to-be-included " << implType->second->getNameAsString()
                              << std::endl;
                }
            );
            return;
        }
        auto& paramClassFilePath{*paramClassFilePathOpt};
        auto projectRoot{std::filesystem::current_path().string()};
        if (projectRoot != paramClassFilePath)
        {
            // Find the relative path to include.
            namespace lsp = llvm::sys::path;
            const auto thisPathEnd = lsp::end(projectRoot);
            const auto paramPathEnd = lsp::end(paramClassFilePath);
            std::optional<std::string> includeOpt;
            for (auto thisPathIt = lsp::begin(projectRoot), paramPathIt = lsp::begin(paramClassFilePath);
                 thisPathIt != thisPathEnd && paramPathIt != paramPathEnd;)
            {
                auto& thisSegment{*thisPathIt};
                auto& paramSegment{*paramPathIt};
                bool startPath = false;
                if (thisSegment != paramSegment)
                {
                    startPath = true;
                }
                else
                {
                    auto nextThis = thisPathIt;
                    ++nextThis;
                    if (nextThis == thisPathEnd)
                    {
                        startPath = true;
                        ++paramPathIt;
                    }
                }
                if (startPath)
                {
                    debugPrint(
                        [&]()
                        {
                            std::cout << "Paths diverged when this=" << thisSegment.str()
                                      << " include=" << paramSegment.str() << " includePath=" << paramClassFilePath
                                      << " workingDir=" << std::filesystem::current_path() << std::endl;
                        }
                    );
                    std::string include;
                    auto backPathIt = thisPathIt;
                    ++backPathIt;
                    // Try to add ../
                    for (; backPathIt != thisPathEnd; ++backPathIt)
                    {
                        debugPrint([&]() { std::cout << "Adding .. for " << backPathIt->str() << std::endl; });
                        include += "..";
                        include += lsp::get_separator();
                    }
                    for (auto includeIt = paramPathIt;;)
                    {
                        include += *includeIt;
                        ++includeIt;
                        if (includeIt == paramPathEnd)
                        {
                            break;
                        }
                        else
                        {
                            include += lsp::get_separator();
                        }
                    }
                    includeOpt.emplace(std::move(include));
                    break;
                }
                ++thisPathIt;
                ++paramPathIt;
            }
            if (!includeOpt)
            {
                debugPrint(
                    [&]() {
                        std::cout << "Failed to generate include for " << implType->second->getNameAsString()
                                  << std::endl;
                    }
                );
                return;
            }
            auto& include{*includeOpt};
            auto extension{llvm::sys::path::extension(include)};
            if (auto extensionIndex = include.rfind(extension); extensionIndex != std::string::npos)
            {
                include.erase(extensionIndex);
            }
            includes.insert(include + ".idl");
        }
    }
}

std::unordered_map<std::string, std::string> idlgen::RuntimeClassVisitor::initCxxTypeToWinRtTypeMap()
{
    // See https://learn.microsoft.com/en-us/uwp/midl-3/intro#types
    return {
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

idlgen::GetMethodResponse idlgen::RuntimeClassVisitor::GetMethods(clang::CXXRecordDecl* record, bool isPropertyDefault)
{
    std::map<std::string, MethodHolder> methodHolders;
    std::map<std::string, clang::CXXMethodDecl*> events;
    std::set<clang::CXXMethodDecl*> ctors;
    auto tryAddMethod =
        [&](clang::CXXMethodDecl* method, std::string name, bool isPropertyDefault, bool isStatic, bool isProtected)
    {
        debugPrint([&]() { std::cout << "Checking if " << name << " is runtime class method" << std::endl; });
        assert(method != nullptr);
        auto methodKind{GetRuntimeClassMethodKind(isPropertyDefault, method)};
        if (!methodKind)
        {
            return;
        }
        if (IsDestructor(method))
        {
            return;
        }
        if (IsConstructor(method))
        {
            ctors.insert(method);
            return;
        }
        if (IsEventRevoker(method))
        {
            return;
        }
        if (IsEventRegistrar(method))
        {
            events.insert({std::move(name), method});
            return;
        }
        debugPrint([&]() { std::cout << name << " is a runtime class method/prop" << std::endl; });
        auto& group{GetOrCreateMethodGroup(methodHolders, method, *methodKind, std::move(name), isStatic, isProtected)};
        if (methodKind == idlgen::MethodKind::Setter)
        {
            group.setter = method;
        }
        else if (methodKind == idlgen::MethodKind::Getter)
        {
            group.getter = method;
        }
        else
        {
            group.method = method;
        }
    };
    auto methods{record->methods()};
    for (auto&& method : methods)
    {
        tryAddMethod(
            method,
            method->getNameAsString(),
            isPropertyDefault,
            method->isStatic(),
            method->getAccess() == clang::AccessSpecifier::AS_protected
        );
    }
    auto handleDataMember = [&](clang::ValueDecl* dataMember, bool isStatic, bool isProtected)
    {
        debugPrint(
            [&]()
            {
                std::cout << dataMember->getNameAsString() << " is data member isStatic=" << isStatic
                          << " isProtected=" << isProtected << std::endl;
            }
        );
        auto fieldKind{GetRuntimeClassFieldKind(isPropertyDefault, dataMember)};
        if (!fieldKind)
        {
            return;
        }
        auto type{dataMember->getType()};
        auto printer{GetMethodPrinter(dataMember, type, isStatic, isProtected)};
        if (printer != nullptr)
        {
            methodHolders.insert({dataMember->getNameAsString(), MethodHolder{std::move(printer), std::nullopt}});
            return;
        }
        auto typeRecord{type->getAsCXXRecordDecl()};
        if (typeRecord == nullptr)
        {
            debugPrint([&]()
                       { std::cout << dataMember->getNameAsString() << "'s type is not a CXXRecord" << std::endl; });
            return;
        }
        // Check if the type has overloaded operator()
        // TODO: Should check for all callable? e.g. std::function
        const auto isFieldPropertyDefault{*fieldKind == idlgen::FieldKind::PropertyDefault};
        ForThisAndBaseMethods(
            typeRecord,
            [&](clang::CXXMethodDecl* fieldMethod)
            {
                if (!GetRuntimeClassMethodKind(isFieldPropertyDefault, fieldMethod))
                {
                    return;
                }
                if (fieldMethod->getNameAsString() != "operator()")
                {
                    return;
                }
                tryAddMethod(fieldMethod, dataMember->getNameAsString(), isFieldPropertyDefault, isStatic, isProtected);
            }
        );
    };
    auto fields{record->fields()};
    for (auto&& field : fields)
    {
        handleDataMember(field, false, field->getAccess() == clang::AccessSpecifier::AS_protected);
    }
    auto decls{record->decls()};
    for (auto&& decl : decls)
    {
        auto varDecl{clang::dyn_cast<clang::VarDecl>(decl)};
        if (varDecl == nullptr)
        {
            continue;
        }
        handleDataMember(
            varDecl, varDecl->isStaticDataMember(), varDecl->getAccess() == clang::AccessSpecifier::AS_protected
        );
    }
    return {std::move(methodHolders), std::move(events), std::move(ctors)};
}

std::string idlgen::RuntimeClassVisitor::TranslateCxxTypeToWinRtType(clang::QualType type)
{
    auto nonRefType{StripReference(type)};
    if (type->isVoidType())
    {
        return "void";
    }
    else if (nonRefType->isBooleanType())
    {
        return "Boolean";
    }
    std::string qualifiedName;
    clang::NamedDecl* decl{nullptr};
    auto record{StripReferenceAndGetClassDecl(type)};
    if (record != nullptr)
    {
        llvm::raw_string_ostream typeOs{qualifiedName};
        record->printQualifiedName(typeOs);
        decl = record;
    }
    else if (nonRefType->isScalarType())
    {
        if (nonRefType->isEnumeralType())
        {
            auto enumType{clang::cast<clang::EnumType>(nonRefType.getCanonicalType())};
            auto enumDecl{enumType->getDecl()};
            qualifiedName = enumDecl->getQualifiedNameAsString();
            decl = enumDecl;
        }
        else
        {
            qualifiedName = nonRefType.getCanonicalType().getAsString();
        }
    }
    else
    {
        debugPrint([&]() { std::cout << nonRefType.getAsString() << " is not a record nor scalar" << std::endl; });
        return "error-type";
    }
    if (auto result{cxxTypeToWinRtTypeMap.find(qualifiedName)}; result != cxxTypeToWinRtTypeMap.end())
    {
        return result->second;
    }
    // Some WinRT primitives are alias, handle them.
    auto namedDecl{StripReferenceAndGetNamedDecl(type)};
    if (namedDecl != nullptr)
    {
        qualifiedName = namedDecl->getQualifiedNameAsString();
        if (auto result{cxxTypeToWinRtTypeMap.find(qualifiedName)}; result != cxxTypeToWinRtTypeMap.end())
        {
            return result->second;
        }
    }
    if (decl == nullptr)
    {
        debugPrint([&]() { std::cout << qualifiedName << " is not a built-in type nor a name decl" << std::endl; });
        return "error-type";
    }
    auto name{decl->getNameAsString()};
    const auto namespaces{GetWinRtNamespaces(decl)};
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
        auto params{templateSpec->getTemplateArgs().asArray()};
        std::string paramString;
        for (auto&& param : params)
        {
            if (param.getKind() != clang::TemplateArgument::ArgKind::Type)
            {
                continue;
            }
            auto type{param.getAsType()};
            paramString += TranslateCxxTypeToWinRtType(type);
            paramString += ", ";
        }
        if (!paramString.empty())
        {
            const auto expectedLastCommaSpaceIndex = paramString.size() - 2;
            const auto lastCommaSpaceIndex = paramString.rfind(", ");
            if (lastCommaSpaceIndex == expectedLastCommaSpaceIndex)
            {
                paramString.erase(expectedLastCommaSpaceIndex);
            }
            qualifiedWinRtName += "<";
            qualifiedWinRtName += paramString;
            if (qualifiedWinRtName.back() == '>')
            {
                qualifiedWinRtName += " ";
            }
            qualifiedWinRtName += ">";
        }
    }
    return qualifiedWinRtName;
}

bool idlgen::RuntimeClassVisitor::IsCppWinRtPrimitive(std::string const& type)
{
    return type == "winrt::hstring" || type == "winrt::event_token" || type == "winrt::Windows::Foundation::DateTime" ||
           type == "winrt::Windows::Foundation::TimeSpan" || type == "winrt::Windows::Foundation::IInspectable";
}

bool idlgen::RuntimeClassVisitor::IsRuntimeClassMethodType(clang::QualType type, bool projectedOnly)
{
    auto nonRefType{StripReference(type)};
    if (nonRefType->isVoidType())
    {
        return true;
    }
    if (nonRefType->isScalarType())
    {
        auto scalarType{nonRefType->getScalarTypeKind()};
        return scalarType == clang::Type::STK_Integral || scalarType == clang::Type::STK_Bool ||
               scalarType == clang::Type::STK_Floating;
    }
    if (nonRefType->isEnumeralType())
    {
        return true;
    }
    if (nonRefType->isUndeducedAutoType())
    {
        auto autoType{nonRefType->getAs<clang::AutoType>()};
        if (autoType == nullptr)
        {
            debugPrint(
                [&]()
                { std::cout << nonRefType.getUnqualifiedType().getAsString() << " cannot get auto type" << std::endl; }
            );
            return false;
        }
        debugPrint(
            [&]()
            {
                std::cout << nonRefType.getUnqualifiedType().getAsString() << " isDeduced=" << autoType->isDeduced()
                          << " isSugared=" << autoType->isSugared() << std::endl;
            }
        );
        auto deducedType{autoType->getDeducedType()};
        if (deducedType.isNull())
        {
            debugPrint(
                [&]() {
                    std::cout << nonRefType.getUnqualifiedType().getAsString() << " cannot get deducedType"
                              << std::endl;
                }
            );
            return false;
        }
        return IsRuntimeClassMethodType(deducedType, projectedOnly);
    }
    auto record{StripReferenceAndGetClassDecl(type)};
    if (record == nullptr)
    {
        debugPrint(
            [&]()
            {
                auto rawType{nonRefType.getUnqualifiedType().getAsString()};
                std::cout << rawType << " struct=" << type->isStructureType() << " class=" << type->isClassType()
                          << " incomplete=" << type->isIncompleteType() << " reference=" << type->isReferenceType()
                          << "\n";
            }
        );
        return false;
    }
    if (!record->isCompleteDefinition())
    {
        debugPrint([&]() { std::cout << record->getNameAsString() << " is not complete" << std::endl; });
        // We have to count incomplete type as OK since clang's AST wouldn't make template reference
        // (e.g. TypedHandler<T, A> const&) a complete type but we definitely want to support these
        // class...
        return true;
    }
    if (record->isPOD())
    {
        return true;
    }
    std::string qualifiedName{record->getQualifiedNameAsString()};
    debugPrint([&]() { std::cout << "Checking if cxx record " << qualifiedName << " is primitive" << std::endl; });
    if (IsCppWinRtPrimitive(qualifiedName))
    {
        return true;
    }
    auto kindOpt = GetRuntimeClassKind(record);
    if (kindOpt && (!projectedOnly || *kindOpt == idlgen::RuntimeClassKind::Projected))
    {
        return true;
    }
    // Some WinRT primitives are alias, handle them.
    // TODO: Make all name-based logic recusrively check typedef
    auto namedDecl{StripReferenceAndGetNamedDecl(type)};
    if (namedDecl == nullptr)
    {
        debugPrint([&]() { std::cout << record->getNameAsString() << " is not a named declaration" << std::endl; });
        return false;
    }
    qualifiedName = namedDecl->getQualifiedNameAsString();
    debugPrint([&]() { std::cout << "Checking if named decl " << qualifiedName << " is primitive" << std::endl; });
    return IsCppWinRtPrimitive(qualifiedName);
}

bool idlgen::RuntimeClassVisitor::IsEventRevoker(clang::CXXMethodDecl* method)
{
    auto params{method->parameters()};
    if (params.size() != 1)
    {
        return false;
    }
    auto record = StripReferenceAndGetClassDecl(params.front()->getType());
    if (record == nullptr)
    {
        return false;
    }
    return GetQualifiedName(record) == "winrt::event_token";
}

bool idlgen::RuntimeClassVisitor::IsEventRegistrar(clang::CXXMethodDecl* method)
{
    auto returnRecord = method->getReturnType()->getAsCXXRecordDecl();
    if (returnRecord == nullptr)
    {
        return false;
    }
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

bool idlgen::RuntimeClassVisitor::ShouldSkipGenerating(clang::NamedDecl* decl)
{
    // We skip checking whether the file contains impl class for built-in headers.
    // TODO: Outright skip parsing them?
    auto fileNameOpt{GetLocFilePath(decl)};
    if (!fileNameOpt)
    {
        return true;
    }
    auto& fileName{*fileNameOpt};
    if (fileName.find("winrt/") != std::string::npos || fileName.find("Microsoft Visual Studio") != std::string::npos ||
        fileName.find("Windows Kits") != std::string::npos)
    {
        return true;
    }
    return false;
}

std::optional<idlgen::MethodKind> idlgen::RuntimeClassVisitor::GetRuntimeClassMethodKind(
    bool isPropertyDefault, clang::CXXMethodDecl* method
)
{
    auto resultGetter = [&]() -> std::optional<idlgen::MethodKind>
    {
        const auto access{method->getAccess()};
        if (!(access == clang::AccessSpecifier::AS_public || access == clang::AccessSpecifier::AS_protected))
        {
            return std::nullopt;
        }
        auto methodAttrs{method->attrs()};
        auto isProperty{isPropertyDefault};
        for (auto&& attr : methodAttrs)
        {
            auto idlGenAttr = GetIdlGenAttr(attr);
            if (!idlGenAttr)
            {
                continue;
            }
            if (idlGenAttr->type == IdlGenAttrType::Hide)
            {
                return std::nullopt;
            }
            else if (idlGenAttr->type == IdlGenAttrType::Property)
            {
                isProperty = true;
            }
            else if (idlGenAttr->type == IdlGenAttrType::Method)
            {
                isProperty = false;
            }
        }
        auto params{method->parameters()};
        auto returnType{method->getReturnType()};
        if (isProperty && params.size() == 0)
        {
            return returnType->isVoidType()               ? std::optional(idlgen::MethodKind::Method)
                   : IsRuntimeClassMethodType(returnType) ? std::optional(idlgen::MethodKind::Getter)
                                                          : std::nullopt;
        }
        if (isProperty && params.size() == 1)
        {
            auto paramType{params[0]->getType()};
            return IsRuntimeClassMethodType(returnType) && IsRuntimeClassMethodType(paramType, true)
                       ? std::optional(idlgen::MethodKind::Setter)
                       : std::nullopt;
        }
        // Ordinary methods
        if (!IsRuntimeClassMethodType(returnType))
        {
            return std::nullopt;
        }
        for (auto&& param : params)
        {
            auto type{param->getType()};
            if (!IsRuntimeClassMethodType(type, true))
            {
                return std::nullopt;
            }
        }
        return idlgen::MethodKind::Method;
    };
    auto result{resultGetter()};
    if (result)
    {
        FindFileToInclude(method->getReturnType());
        auto params{method->parameters()};
        for (auto&& param : params)
        {
            FindFileToInclude(param->getType());
        }
    }
    return result;
}

std::optional<idlgen::FieldKind> idlgen::RuntimeClassVisitor::GetRuntimeClassFieldKind(
    bool isPropertyDefault, clang::ValueDecl* value
)
{
    auto resultGetter = [&]() -> std::optional<idlgen::FieldKind>
    {
        bool isFieldPropertyDefault{isPropertyDefault};
        auto fieldAttrs{value->attrs()};
        const auto access{value->getAccess()};
        if (!(access == clang::AccessSpecifier::AS_public || access == clang::AccessSpecifier::AS_protected))
        {
            return std::nullopt;
        }
        for (auto&& fieldAttr : fieldAttrs)
        {
            auto idlgenAttr{GetIdlGenAttr(fieldAttr)};
            if (!idlgenAttr)
            {
                continue;
            }
            if (idlgenAttr->type == idlgen::IdlGenAttrType::Hide)
            {
                return std::nullopt;
            }
            if (idlgenAttr->type == idlgen::IdlGenAttrType::Property)
            {
                isFieldPropertyDefault = true;
            }
            else if (idlgenAttr->type == idlgen::IdlGenAttrType::Method)
            {
                isFieldPropertyDefault = false;
            }
        }
        return isFieldPropertyDefault ? idlgen::FieldKind::PropertyDefault : idlgen::FieldKind::MethodDefault;
    };
    auto result{resultGetter()};
    if (result)
    {
        FindFileToInclude(value->getType());
    }
    return result;
}

clang::QualType idlgen::RuntimeClassVisitor::StripReference(clang::QualType type)
{
    auto nonRef{type->isReferenceType() ? type.getNonReferenceType() : type};
    if (nonRef.isLocalConstQualified())
    {
        nonRef.removeLocalConst();
    }
    return nonRef;
}

clang::CXXRecordDecl* idlgen::RuntimeClassVisitor::StripReferenceAndGetClassDecl(clang::QualType type)
{
    return type->isReferenceType() ? type.getNonReferenceType()->getAsCXXRecordDecl() : type->getAsCXXRecordDecl();
}

const clang::NamedDecl* idlgen::RuntimeClassVisitor::StripReferenceAndGetNamedDecl(clang::QualType type)
{
    const clang::Type* typePtr;
    if (type->isReferenceType())
    {
        typePtr = type.getNonReferenceType().getTypePtrOrNull();
    }
    else
    {
        typePtr = type.getTypePtrOrNull();
    }
    if (typePtr->isNullPtrType())
    {
        return nullptr;
    }
    auto typedefType{typePtr->getAs<clang::TypedefType>()};
    if (typedefType != nullptr)
    {
        return typedefType->getDecl();
    }
    return typePtr->getAsTagDecl();
}

std::optional<idlgen::RuntimeClassKind> idlgen::RuntimeClassVisitor::GetRuntimeClassKind(clang::QualType type)
{
    auto record{StripReferenceAndGetClassDecl(type)};
    if (record == nullptr)
    {
        return std::nullopt;
    }
    return GetRuntimeClassKind(record);
}

std::optional<idlgen::EnumKind> idlgen::RuntimeClassVisitor::GetEnumKind(clang::EnumDecl* decl)
{
    if (!decl->isComplete())
    {
        return std::nullopt;
    }
    if (!decl->isScoped())
    {
        return std::nullopt;
    }
    auto type{decl->getIntegerType()};
    if (type.isNull())
    {
        return std::nullopt;
    }
    auto typedefType{type->getAs<clang::TypedefType>()};
    if (typedefType == nullptr)
    {
        return std::nullopt;
    }
    auto name{typedefType->getDecl()->getQualifiedNameAsString()};
    debugPrint([&]() { std::cout << decl->getNameAsString() << "'s underlying's name is " << name << std::endl; });
    if (name == nameAuthorEnum)
    {
        return EnumKind::Normal;
    }
    else if (name == nameAuthorEnumFlags)
    {
        return EnumKind::Flag;
    }
    return std::optional<EnumKind>();
}

std::optional<idlgen::RuntimeClassKind> idlgen::RuntimeClassVisitor::GetRuntimeClassKind(
    clang::CXXRecordDecl* record, bool implementationOnly
)
{
    auto className{record->getNameAsString()};
    debugPrint([&]() { std::cout << "Checking if " << className << " is runtime class" << std::endl; });
    auto filePathOpt{GetLocFilePath(record)};
    if (implementationOnly)
    {
        auto parentContext{record->getParent()};
        if (parentContext == nullptr)
        {
            return std::nullopt;
        }
        if (!parentContext->isNamespace())
        {
            return std::nullopt;
        }
        auto namespaceDecl = static_cast<clang::NamespaceDecl*>(parentContext);
        if (namespaceDecl->getNameAsString() != "implementation")
        {
            return std::nullopt;
        }
    }
    if (!record->isCompleteDefinition())
    {
        debugPrint([&]() { std::cout << className << " is not complete" << std::endl; });
        return std::nullopt;
    }
    auto bases{record->bases()};
    for (auto&& base : bases)
    {
        debugPrint([&]() { std::cout << "Checking base " << base.getType().getAsString() << std::endl; });
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
        // TODO: Need to check full name as Windows::Foundation::IInspectable instead
        if (cxxType->getName() == "IInspectable" || cxxType->getName() == "IUnknown")
        {
            return idlgen::RuntimeClassKind::Projected;
        }
        auto templateSpecType{baseType->getAs<clang::TemplateSpecializationType>()};
        auto spec{clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(cxxType)};
        if (templateSpecType != nullptr && spec != nullptr)
        {
            auto templateDecl{templateSpecType->getTemplateName().getAsTemplateDecl()};
            auto templateName{templateDecl->getNameAsString()};
            debugPrint([&]() { std::cout << templateName << " is a template specialization" << std::endl; });
            auto params{spec->getTemplateArgs().asArray()};
            for (auto&& param : params)
            {
                auto paramKind = param.getKind();
                if (paramKind != clang::TemplateArgument::ArgKind::Type)
                {
                    debugPrint(
                        [&]()
                        {
                            std::cout << "Template param ";
                            param.print(clang::LangOptions(), llvm::outs(), true);
                            std::cout << " is not a type" << std::endl;
                        }
                    );
                    continue;
                }
                auto type{param.getAsType()->getAsCXXRecordDecl()};
                if (type == nullptr)
                {
                    debugPrint([&]()
                               { std::cout << param.getAsType().getAsString() << " is not a CXXRecord" << std::endl; });
                    continue;
                }
                auto templateParamTypeName{type->getNameAsString()};
                auto expectedParamTypeName{std::string_view(templateName).substr(0, templateName.size() - 1)};
                debugPrint(
                    [&]()
                    {
                        std::cout << "templateParamTypeName=" << templateParamTypeName << " className=" << className
                                  << " templateName=" << templateName
                                  << " expectedParamTypeName=" << expectedParamTypeName
                                  << " cxxTypeName=" << cxxType->getName().data() << std::endl;
                    }
                );
                if (templateParamTypeName == expectedParamTypeName && templateParamTypeName == className)
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

std::optional<std::vector<clang::QualType>> idlgen::RuntimeClassVisitor::GetExtend(clang::CXXRecordDecl* record)
{
    debugPrint([&]() { std::cout << "Getting extend for " << record->getNameAsString() << std::endl; });
    if (!record->isCompleteDefinition())
    {
        return std::nullopt;
    }
    auto bases{record->bases()};
    std::vector<clang::QualType> result;
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
        auto spec{clang::dyn_cast<clang::ClassTemplateSpecializationDecl>(cxxType)};
        if (spec == nullptr)
        {
            continue;
        }
        auto typeName{GetQualifiedName(cxxType)};
        if (typeName != "idlgen::base")
        {
            continue;
        }
        auto templateParams{spec->getTemplateArgs().asArray()};
        auto tryAddTypeToResult = [&](clang::QualType type)
        {
            if (!IsRuntimeClassMethodType(type, true))
            {
                return;
            }
            auto paramCxxType{type->getAsCXXRecordDecl()};
            if (paramCxxType == nullptr)
            {
                debugPrint([&]() { std::cout << type.getAsString() << " is not a CXXRecord" << std::endl; });
                return;
            }
            debugPrint([&]() { std::cout << paramCxxType->getNameAsString() << " is an extend base" << std::endl; });
            // TODO: Verify first type is projected type, second is interface (how?)
            result.emplace_back(type);
        };
        for (auto&& templateParam : templateParams)
        {
            auto paramKind = templateParam.getKind();
            if (paramKind == clang::TemplateArgument::ArgKind::Type)
            {
                auto paramType{templateParam.getAsType()};
                tryAddTypeToResult(paramType);
            }
            else if (paramKind == clang::TemplateArgument::ArgKind::Pack)
            {
                auto packParams{templateParam.getPackAsArray()};
                for (auto&& packParam : packParams)
                {
                    auto paramType{packParam.getAsType()};
                    tryAddTypeToResult(paramType);
                }
            }
            else
            {
                debugPrint(
                    [&]()
                    {
                        std::cout << "Template param ";
                        templateParam.print(clang::LangOptions(), llvm::outs(), true);
                        std::cout << " is not a type but " << paramKind << std::endl;
                    }
                );
            }
        }
    }
    if (result.empty())
    {
        return std::nullopt;
    }
    return result;
}

std::vector<std::string> idlgen::RuntimeClassVisitor::GetWinRtNamespaces(clang::NamedDecl* record)
{
    std::vector<std::string> namespaces;
    auto parentContext = record->getDeclContext();
    while (parentContext != nullptr)
    {
        if (!parentContext->isNamespace())
        {
            break;
        }
        auto namespaceDecl = static_cast<clang::NamespaceDecl*>(parentContext);
        auto name{namespaceDecl->getNameAsString()};
        if (!(name == "winrt" || name == "implementation"))
        {
            namespaces.emplace_back(std::move(name));
        }
        parentContext = parentContext->getParent();
    }
    std::reverse(namespaces.begin(), namespaces.end());
    return namespaces;
}

std::string idlgen::RuntimeClassVisitor::GetQualifiedName(clang::CXXRecordDecl* record)
{
    std::string qualifiedName;
    llvm::raw_string_ostream typeOs{qualifiedName};
    record->printQualifiedName(typeOs);
    return qualifiedName;
}

clang::QualType idlgen::RuntimeClassVisitor::GetFirstTemplateTypeParam(
    clang::ClassTemplateSpecializationDecl const* templateSpecDecl
)
{
    auto params{templateSpecDecl->getTemplateArgs().asArray()};
    std::optional<clang::QualType> propertyTypeOpt;
    for (auto&& param : params)
    {
        auto paramKind = param.getKind();
        if (paramKind != clang::TemplateArgument::ArgKind::Type)
        {
            debugPrint(
                [&]()
                {
                    std::cout << "Template param ";
                    param.print(clang::LangOptions(), llvm::outs(), true);
                    std::cout << " is not a type" << std::endl;
                }
            );
            continue;
        }
        return param.getAsType();
    }
    return clang::QualType();
}

std::optional<std::string> idlgen::RuntimeClassVisitor::GetLocFilePath(clang::NamedDecl* decl)
{
    auto& srcManager{astContext.getSourceManager()};
    auto file{srcManager.getFileEntryForID(srcManager.getFileID(decl->getLocation()))};
    if (file == nullptr)
    {
        return std::nullopt;
    }
    return file->tryGetRealPathName().str();
}

std::optional<std::string> idlgen::RuntimeClassVisitor::GetLocFileName(clang::CXXRecordDecl* record)
{
    auto& srcManager{astContext.getSourceManager()};
    auto file{srcManager.getFileEntryForID(srcManager.getFileID(record->getLocation()))};
    if (file == nullptr)
    {
        return std::nullopt;
    }
    return llvm::sys::path::filename(file->getName()).str();
}

void idlgen::RuntimeClassVisitor::PrintNameSpaces(std::vector<std::string> namespaces)
{
    out << "namespace ";
    const auto namespaceCount = namespaces.size();
    for (size_t i = 0; i < namespaceCount; ++i)
    {
        out << namespaces[i];
        if (i + 1 < namespaceCount)
        {
            out << ".";
        }
    }
}

void idlgen::RuntimeClassVisitor::PrintMethodParams(clang::CXXMethodDecl* method)
{
    out << "(";
    auto params{method->parameters()};
    const auto paramCount = params.size();
    for (size_t i = 0; i < paramCount; ++i)
    {
        auto param{params[i]};
        out << TranslateCxxTypeToWinRtType(param->getType()) << " " << param->getNameAsString();
        if (i + 1 < paramCount)
        {
            out << ", ";
        }
    }
    out << ");";
}

void idlgen::RuntimeClassVisitor::PrintEvent(std::string_view name, clang::CXXMethodDecl* method)
{
    assert(method->parameters().size() > 0);
    auto handler{TranslateCxxTypeToWinRtType(method->parameters().front()->getType())};
    out << "event " << handler << " " << name << ";"
        << "\n";
}

std::unique_ptr<idlgen::Printer> idlgen::RuntimeClassVisitor::TryHandleAsClass(
    clang::CXXRecordDecl* decl, bool isPropertyDefault, std::vector<idlgen::IdlGenAttr>& attrs
)
{
    if (!GetRuntimeClassKind(decl))
    {
        return nullptr;
    }
    auto response{GetMethods(decl, isPropertyDefault)};
    auto& methodHolders{response.holders};
    auto& events{response.events};
    auto& ctors{response.ctors};
    std::optional<std::vector<clang::QualType>> extend{GetExtend(decl)};
    debugPrint([&]() { std::cout << decl->getNameAsString() << " propertyDefault=" << isPropertyDefault << std::endl; }
    );
    // Add default_interface if the runtime class is empty
    if (methodHolders.empty())
    {
        attrs.emplace_back(IdlGenAttr{IdlGenAttrType::Attribute, {"default_interface"}});
    }
    return std::make_unique<idlgen::ClassPrinter>(decl, std::move(response), std::move(extend));
}

std::unique_ptr<idlgen::Printer> idlgen::RuntimeClassVisitor::TryHandleAsInterface(
    clang::CXXRecordDecl* decl, bool isPropertyDefault
)
{
    if (!IsSingleBaseOfType(decl, "idlgen::author_interface"))
    {
        return nullptr;
    }
    auto methodResponse{GetMethods(decl, isPropertyDefault)};
    return std::make_unique<InterfacePrinter>(decl, std::move(methodResponse));
}

std::unique_ptr<idlgen::DelegatePrinter> idlgen::RuntimeClassVisitor::TryHandleAsDelegate(clang::CXXRecordDecl* decl)
{
    if (!IsSingleBaseOfType(decl, "idlgen::author_delegate"))
    {
        return nullptr;
    }
    clang::CXXMethodDecl* method{nullptr};
    auto methods{decl->methods()};
    for (auto&& candidateMethod : methods)
    {
        debugPrint(
            [&]() { std::cout << "Finding delegate signature from" << candidateMethod->getNameAsString() << std::endl; }
        );
        if (method != nullptr)
        {
            return nullptr;
        }
        if (GetRuntimeClassMethodKind(false, candidateMethod) != MethodKind::Method)
        {
            return nullptr;
        }
        if (candidateMethod->param_size() != 2)
        {
            return nullptr;
        }
        method = candidateMethod;
    }
    if (method == nullptr)
    {
        return nullptr;
    }
    if (method->getNameAsString() != "operator()")
    {
        return nullptr;
    }
    return std::make_unique<idlgen::DelegatePrinter>(decl, method);
}

std::unique_ptr<idlgen::Printer> idlgen::RuntimeClassVisitor::TryHandleAsStruct(clang::CXXRecordDecl* decl)
{
    if (!IsSingleBaseOfType(decl, nameAuthorStruct))
    {
        return nullptr;
    }
    auto fields{decl->fields()};
    std::vector<clang::FieldDecl*> validFields;
    for (auto&& field : fields)
    {
        auto fieldKind{GetRuntimeClassFieldKind(true, field)};
        if (!fieldKind)
        {
            continue;
        }
        auto fieldType = field->getType();
        if (!IsRuntimeClassMethodType(fieldType, true))
        {
            continue;
        }
        validFields.emplace_back(field);
    }
    return std::make_unique<idlgen::StructPrinter>(decl, validFields);
}

bool idlgen::RuntimeClassVisitor::IsSingleBaseOfType(clang::CXXRecordDecl* decl, std::string_view name)
{
    if (!decl->isCompleteDefinition())
    {
        return false;
    }
    if (decl->getNumBases() != 1)
    {
        return false;
    }
    auto baseIt{decl->bases_begin()};
    auto& base = *baseIt;
    auto baseType{base.getType().getTypePtrOrNull()};
    if (baseType == nullptr)
    {
        return false;
    }
    auto cxxBase{baseType->getAsCXXRecordDecl()};
    if (cxxBase == nullptr)
    {
        return false;
    }
    auto baseQualifiedName{cxxBase->getQualifiedNameAsString()};
    return baseQualifiedName == name;
}

idlgen::MethodGroup::MethodGroup(
    std::string methodName,
    clang::CXXMethodDecl* method,
    clang::CXXMethodDecl* getter,
    clang::CXXMethodDecl* setter,
    bool isStatic,
    bool isProtected
)
    : methodName(std::move(methodName)), isStatic(isStatic), isProtected(isProtected), method(method), getter(getter),
      setter(setter)
{
}

void idlgen::MethodGroup::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    if (isProtected)
    {
        out << "protected ";
    }
    if (isStatic)
    {
        out << "static ";
    }
    auto returnType{visitor.TranslateCxxTypeToWinRtType(GetterOrElse()->getReturnType())};
    out << returnType << " " << methodName;
    if (IsGetter())
    {
        out << "{get;};";
    }
    else if (IsProperty())
    {
        out << ";";
    }
    else
    {
        visitor.PrintMethodParams(SetterOrElse());
    }
}

idlgen::PropertyMethodPrinter::PropertyMethodPrinter(
    std::string methodName, clang::QualType type, PropertyKind kind, bool isStatic, bool isProtected
)
    : methodName(std::move(methodName)), type(type), kind(kind), isStatic(isStatic), isProtected(isProtected)
{
}

void idlgen::PropertyMethodPrinter::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    if (isProtected)
    {
        out << "protected ";
    }
    if (isStatic)
    {
        out << "static ";
    }
    auto returnType{visitor.TranslateCxxTypeToWinRtType(type)};
    out << returnType << " " << methodName;
    if (kind == PropertyKind::Getter)
    {
        out << "{get;};";
    }
    else
    {
        out << ";";
    }
}

idlgen::DelegatePrinter::DelegatePrinter(clang::CXXRecordDecl* record, clang::CXXMethodDecl* method)
    : record(record), method(method)
{
}

void idlgen::DelegatePrinter::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    out << "delegate ";
    out << visitor.TranslateCxxTypeToWinRtType(method->getReturnType()) << " ";
    out << record->getNameAsString();
    visitor.PrintMethodParams(method);
    out << "\n";
}

idlgen::StructPrinter::StructPrinter(clang::CXXRecordDecl* record, std::vector<clang::FieldDecl*> fields)
    : record(record), fields(std::move(fields))
{
}

void idlgen::StructPrinter::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    out << "struct " << record->getNameAsString() << "\n";
    out << "{\n";
    for (auto&& field : fields)
    {
        auto fieldType = field->getType();
        auto typeName{visitor.TranslateCxxTypeToWinRtType(fieldType)};
        out << typeName << " ";
        out << field->getNameAsString() << ";\n";
    }
    out << "};\n";
}

idlgen::ClassPrinter::ClassPrinter(
    clang::CXXRecordDecl* record, GetMethodResponse response, std::optional<std::vector<clang::QualType>> extend
)
    : record(record), response(std::move(response)), extend(std::move(extend))
{
}

void idlgen::ClassPrinter::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    out << "runtimeclass " << record->getNameAsString();
    auto& holders{response.holders};
    auto& events{response.events};
    auto& ctors{response.ctors};
    if (extend)
    {
        visitor.debugPrint([&]() { std::cout << "extend size is " << extend->size() << std::endl; });
        if (!extend->empty())
        {
            out << " : ";
        }
        auto& bases{*extend};
        const auto baseCount = bases.size();
        for (size_t i = 0; i < baseCount; ++i)
        {
            auto& base = bases[i];
            auto name{visitor.TranslateCxxTypeToWinRtType(base)};
            out << name;
            if (i + 1 < baseCount)
            {
                out << ", ";
            }
        }
    }
    out << "\n";
    out << "{"
        << "\n";
    for (auto&& ctor : ctors)
    {
        out << ctor->getNameAsString();
        visitor.PrintMethodParams(ctor);
        out << "\n";
    }
    for (auto&& entry : holders)
    {
        auto& group{entry.second};
        group.printer->Print(visitor, out);
        out << "\n";
    }
    for (auto&& entry : events)
    {
        auto& name{entry.first};
        auto& ev{entry.second};
        visitor.PrintEvent(name, ev);
    }
    out << "}\n";
}

idlgen::InterfacePrinter::InterfacePrinter(clang::CXXRecordDecl* record, GetMethodResponse response)
    : record(record), response(std::move(response))
{
}

void idlgen::InterfacePrinter::Print(RuntimeClassVisitor& visitor, llvm::raw_ostream& out)
{
    auto& holders{response.holders};
    auto& events{response.events};
    out << "interface " << record->getNameAsString() << "\n";
    out << "{\n";
    for (auto&& holder : holders)
    {
        holder.second.printer->Print(visitor, out);
    }
    for (auto&& ev : events)
    {
        visitor.PrintEvent(ev.first, ev.second);
    }
    out << "}\n";
}
