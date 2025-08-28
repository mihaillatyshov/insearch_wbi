#pragma once

#include <format>
#include <string>

namespace LM
{

    class FileFormat
    {
    public:
        static std::string FormatId(int _PageId) { return std::format("{:0>4}", _PageId); }
        static std::string FormatId(size_t _PageId) { return std::format("{:0>4}", _PageId); }
        static std::string FormatImg(int _PageId) { return FormatId(_PageId) + ".png"; }
        static std::string FormatXlsx(int _PageId) { return FormatId(_PageId) + ".xlsx"; }
    };

}    // namespace LM
