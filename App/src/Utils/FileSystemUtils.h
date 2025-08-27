#pragma once

#include <filesystem>
#include <string>

namespace LM
{

    class FileSystemUtils
    {
    public:
        static void RemoveAllInFolder(std::string _Folder)
        {
            for (const auto& entry : std::filesystem::directory_iterator(_Folder))
            {
                std::filesystem::remove_all(entry.path());
            }
        }

        static size_t FilesCountInDirectory(std::filesystem::path path)
        {
            return std::distance(std::filesystem::directory_iterator(path), std::filesystem::directory_iterator {});
        }
    };

}    // namespace LM
