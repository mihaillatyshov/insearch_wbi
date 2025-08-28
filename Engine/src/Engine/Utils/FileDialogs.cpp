#include "FileDialogs.h"

#include "Engine/Utils/Log.hpp"

#include <filesystem>

namespace LM
{

    std::string FileDialogs::OpenFile(const std::vector<nfdfilteritem_t>& _Filter)
    {
        LOG_CORE_WARN("{}", std::filesystem::current_path().string().c_str());

        std::string currentPath = std::filesystem::current_path().string();

        NFD::UniquePath outPath;
        nfdresult_t result = NFD::OpenDialog(outPath, _Filter.data(), static_cast<nfdfiltersize_t>(_Filter.size()),
                                             static_cast<const nfdchar_t*>(currentPath.c_str()));

        if (result != NFD_OKAY)
        {
            if (result == NFD_CANCEL)
            {
                LOG_CORE_ERROR("User canceled the dialog.");
            }
            else
            {
                LOG_CORE_ERROR("Error: {}", NFD::GetError());
            }

            return std::string();
        }

        else
        {
            return outPath.get();
        }
    }

    std::vector<std::string> FileDialogs::OpenMultipleFiles(const std::vector<nfdfilteritem_t>& _Filter)
    {
        NFD::UniquePathSet outPaths;
        nfdresult_t result =
            NFD::OpenDialogMultiple(outPaths, _Filter.data(), static_cast<nfdfiltersize_t>(_Filter.size()));

        if (result != NFD_OKAY)
        {
            if (result == NFD_CANCEL)
            {
                LOG_CORE_ERROR("User canceled the dialog.");
            }
            else
            {
                LOG_CORE_ERROR("Error: {}", NFD::GetError());
            }

            return std::vector<std::string>();
        }

        nfdpathsetsize_t count = 0;
        NFD::PathSet::Count(outPaths, count);

        std::vector<std::string> paths(count);
        for (nfdpathsetsize_t i = 0; i < count; ++i)
        {
            NFD::UniquePathSetPath path;
            NFD::PathSet::GetPath(outPaths, i, path);
            paths[i] = path.get();
        }

        return paths;
    }

}    // namespace LM
