#pragma once

#include <string>

namespace LM
{

    struct Catalog
    {
        bool NeedRebuild = false;
        bool SplitPages = false;  
        std::string CatalogBaseFileName;
        int ImgQuality = 1;
    };

}
