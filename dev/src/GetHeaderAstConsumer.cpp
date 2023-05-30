#include "GetHeaderAstConsumer.h"
#include "clang/Lex/Preprocessor.h"

idlgen::GetHeaderAstConsumer::GetHeaderAstConsumer(clang::CompilerInstance& ci, std::vector<std::string>& headerFullPaths
) :
    ci(ci),
    headerFullPaths(headerFullPaths)
{
}

void idlgen::GetHeaderAstConsumer::HandleTranslationUnit(clang::ASTContext& context)
{
    auto& includedFiles{ci.getPreprocessor().getIncludedFiles()};
    for (auto&& file : includedFiles)
    {
        if (file == nullptr)
        {
            continue;
        }
        // TODO: Ignore non-source header (e.g. sdk, VS)
        headerFullPaths.emplace_back(file->tryGetRealPathName());
    }
}
