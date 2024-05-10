#pragma once

namespace LM
{

    struct GenRawExcel
    {
        bool UseCutPattern = false;

        bool IsGenerated = false;
        bool NeedRebuild = true;

        int32_t Version = -1;
    };

}    // namespace LM
