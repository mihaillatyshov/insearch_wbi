#pragma once

namespace LM
{

    struct GenImgsByCutPattern
    {
        bool IsGenerated = false;
        bool NeedRebuild = true;

        int32_t Version = -1;
    };

}    // namespace LM
