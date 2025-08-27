#pragma once

#include <string>
#include <vector>

namespace LM
{

    struct ExcelFolderStartupItem
    {
        std::string Path;
        std::string Constr;
    };

    class ExcelFolder
    {
    protected:
        std::vector<ExcelFolderStartupItem> m_ExcelFolderStartupItems;
    };

}    // namespace LM
