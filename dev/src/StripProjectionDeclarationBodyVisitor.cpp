#include "StripProjectionDeclarationBodyVisitor.h"
#include "clang/Basic/SourceManager.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/AST/DeclCXX.h"
#include <string>
#include <iostream>

idlgen::StripProjectionDeclarationBodyVisitor::StripProjectionDeclarationBodyVisitor(
    clang::CompilerInstance& ci, llvm::raw_ostream& out, bool verbose
) :
    ci(ci),
    out(out),
    verbose(verbose)
{
}

void idlgen::StripProjectionDeclarationBodyVisitor::Reset()
{
    out << "#pragma once\n\n";
}

void idlgen::StripProjectionDeclarationBodyVisitor::Finish()
{
    auto& includes{ci.getPreprocessor().getIncludedFiles()};
    for (auto&& include : includes)
    {
        auto filePath{include->tryGetRealPathName()};
        if (filePath.find(".xaml.g.h") != std::string::npos)
        {
            out << "#include \"" << filePath << "\"\n";
        }
    }
}

bool idlgen::StripProjectionDeclarationBodyVisitor::VisitCXXRecordDecl(clang::CXXRecordDecl* record)
{
    auto location{record->getLocation()};
    auto isMain{record->getASTContext().getSourceManager().isInMainFile(location)};
    if (!isMain)
    {
        return true;
    }
    auto recordName{record->getNameAsString()};
    if (auto templateDecl = record->getDescribedTemplate())
    {
        /*using namespace std::string_view_literals;
        constexpr auto runtimeClassBaseLast = "_base"sv;
        if (!llvm::StringRef(recordName).ends_with(runtimeClassBaseLast))
        {
            return true;
        }*/
        out << "namespace ";
        PrintNamespaces(record);
        out << "\n";
        out << "{\n";
        out << "template ";
        out << PrintTemplateParam(templateDecl->getTemplateParameters()->asArray());
        out << "\n";
        if (record->isStruct())
        {
            out << "struct ";
        }
        else
        {
            out << "class ";
        }
        out << recordName << "{};\n";
        out << "};\n";
    }
    return true;
}

bool idlgen::StripProjectionDeclarationBodyVisitor::VisitTypeAliasTemplateDecl(clang::TypeAliasTemplateDecl* d)
{
    // TODO: Refactor to should generate, share with idlgen visitor
    auto location{d->getLocation()};
    auto isMain{d->getASTContext().getSourceManager().isInMainFile(location)};
    if (!isMain)
    {
        return true;
    }
    out << "namespace ";
    PrintNamespaces(d);
    out << "\n";
    out << "{\n";
    auto usingName{d->getNameAsString()};
    out << "template ";
    out << PrintTemplateParam(d->getTemplateParameters()->asArray());
    out << "\n";
    out << "using " << usingName << " = ";
    auto aliasedType{d->getTemplatedDecl()->getUnderlyingType().getTypePtrOrNull()};
    if (aliasedType == nullptr)
    {
        return true;
    }
    auto aliasedTemplateType{aliasedType->getAs<clang::TemplateSpecializationType>()};
    if (aliasedTemplateType == nullptr)
    {
        return true;
    }
    auto aliasedTemplateName{aliasedTemplateType->getTemplateName()};
    if (aliasedTemplateName.isNull())
    {
        return true;
    }
    auto aliasedDecl{aliasedTemplateName.getAsTemplateDecl()};
    if (aliasedDecl == nullptr)
    {
        return true;
    }
    auto aliasedName{aliasedDecl->getNameAsString()};
    out << aliasedName;
    auto templateParams{aliasedTemplateType->template_arguments()};
    std::string paramStr{"<"};
    for (auto&& param : templateParams)
    {
        auto paramType{param.getAsType()};
        if (paramType.isNull())
        {
            continue;
        }
        paramStr += paramType.getAsString() + ", ";
    }
    if (paramStr.size() > 1)
    {
        paramStr.erase(paramStr.size() - 2);
    }
    paramStr += ">";
    out << paramStr << ";\n";
    out << "};\n";
    return true;
}

std::string idlgen::StripProjectionDeclarationBodyVisitor::PrintTemplateParam(llvm::ArrayRef<clang::NamedDecl*> params)
{
    std::string paramStr{"<"};
    for (auto&& param : params)
    {
        if (auto type = clang::dyn_cast<clang::TemplateTypeParmDecl>(param))
        {
            paramStr += "typename";
            if (type->isTemplateParameterPack())
            {
                paramStr += "...";
            }
            paramStr += " ";
        }
        else
        {
            continue;
        }
        auto paramName{param->getNameAsString()};
        paramStr += paramName + ", ";
    }
    if (paramStr.size() > 1)
    {
        paramStr.erase(paramStr.size() - 2);
    }
    paramStr += ">";
    return paramStr;
}

std::vector<std::string> idlgen::StripProjectionDeclarationBodyVisitor::GetNameSpaces(clang::NamedDecl* decl)
{
    std::vector<std::string> namespaces;
    auto parentContext = decl->getDeclContext();
    while (parentContext != nullptr)
    {
        if (!parentContext->isNamespace())
        {
            break;
        }
        auto namespaceDecl = static_cast<clang::NamespaceDecl*>(parentContext);
        auto name{namespaceDecl->getNameAsString()};
        namespaces.emplace_back(std::move(name));
        parentContext = parentContext->getParent();
    }
    std::reverse(namespaces.begin(), namespaces.end());
    return namespaces;
}

void idlgen::StripProjectionDeclarationBodyVisitor::PrintNamespaces(clang::NamedDecl* decl)
{
    auto namespaces{GetNameSpaces(decl)};
    std::string namespaceStr;
    for (auto&& ns : namespaces)
    {
        namespaceStr += ns;
        namespaceStr += "::";
    }
    if (namespaceStr.size() > 0)
    {
        namespaceStr.erase(namespaceStr.size() - 2);
    }
    out << namespaceStr;
}
