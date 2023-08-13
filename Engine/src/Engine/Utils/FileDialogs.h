#pragma once

#include <string>

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
        static std::string OpenFile(const Filter& filter);
        static std::string SaveFile(const Filter& filter);
    };

}    // namespace LM
