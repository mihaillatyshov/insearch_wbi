#pragma once

#include <string>

namespace LM
{

    struct Catalog
    {
        bool NeedRebuild = false;
        bool SplitPages = false;  
        std::string BaseFileName;
        int ImgQuality = 1;
    };

}
