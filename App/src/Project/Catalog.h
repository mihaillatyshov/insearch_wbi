#pragma once

#include "Project/CatalogCutPattern.h"
#include "Project/Processing/GenImgsByCutPattern.h"
#include "Project/Processing/GenRawExcel.h"

#include <string>

namespace LM
{

    struct Catalog
    {
        bool NeedRebuild = false;
        bool SplitPages = false;
        std::string BaseFileName;
        int ImgQuality = 1;
        bool IsGenerated = false;

        CatalogCutPattern TopLeftCutPattern;
        CatalogCutPattern BotRightCutPattern;

        GenImgsByCutPattern GenImgsByCutPattern;
        GenRawExcel GenRawExcel;
        std::vector<uint32_t> GeneratedCatalogExcludePages;

        bool IsNeedRebuild(const Catalog& _LastBuild) const
        {
            return _LastBuild.SplitPages != SplitPages || _LastBuild.BaseFileName != BaseFileName ||
                   _LastBuild.ImgQuality != ImgQuality;
        }

        static Catalog CreateDefaultLastBuildCatalog()
        {
            Catalog result = Catalog();
            result.ImgQuality = -1;
            return result;
        }
    };

}    // namespace LM
