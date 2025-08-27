#pragma once

#include <string>
#include <vector>

#include <nfd.hpp>

namespace LM
{

    class FileDialogs
    {
    public:
        struct Filter
        {
            std::string Description;
            std::string Extention;
        };

        // These return empty strings if cancelled
        static std::string OpenFile(const std::vector<nfdfilteritem_t>& _Filter);
        static std::vector<std::string> OpenMultipleFiles(const std::vector<nfdfilteritem_t>& _Filter);
    };

}    // namespace LM
