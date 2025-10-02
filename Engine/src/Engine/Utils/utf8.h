#pragma once

#include <algorithm>
#include <format>
#include <ranges>
#include <string>
#include <vector>

static std::string operator"" _S(const char8_t* str, std::size_t) { return reinterpret_cast<const char*>(str); }
static const char* operator"" _C(const char8_t* str, std::size_t) { return reinterpret_cast<const char*>(str); }

#if defined(__cpp_char8_t)
    #define _U8_RES(x) x
    #define U8_RES(x)  _U8_RES(u8) _U8_RES(x)##_C
    #define U8(x)      u8##x##_C
    #define U8S(x)     u8##x##_S
#else
    #define U8(x)  u8##x
    #define U8S(x) std::string(u8##x)
#endif

template <typename... Args>
static std::string Format(std::string_view _FmtStr, Args&&... _Args)
{
    return std::vformat(_FmtStr, std::make_format_args(_Args...));
}

inline bool StrContainsNewlineOrTab(std::string_view _Str)
{
    if (_Str.empty())
    {
        return false;
    }

    for (const char* p = _Str.data(); *p != '\0'; ++p)
    {
        if (*p == '\n' || *p == '\t')
        {
            return true;
        }
    }
    return false;
}

inline std::string StrJoin(const std::vector<int>& _Array, const std::string& _Delimiter)
{
    auto str_range = _Array | std::views::transform([](int x) { return std::to_string(x); });

    std::string result;

    if (!_Array.empty())
    {
        result += *str_range.begin();
        std::for_each(std::next(str_range.begin()), str_range.end(), [&](const std::string& s) {
            result += _Delimiter;
            result += s;
        });
    }

    return result;
}

inline std::string StrJoin(const std::vector<std::string>& _Array, const std::string& _Delimiter)
{
    std::string result;

    if (!_Array.empty())
    {
        result += *_Array.begin();
        std::for_each(std::next(_Array.begin()), _Array.end(), [&](const std::string& s) {
            result += _Delimiter;
            result += s;
        });
    }

    return result;
}

inline std::string StrTrim(const std::string& s)
{
    auto start = std::find_if_not(s.begin(), s.end(), [](unsigned char ch) { return std::isspace(ch); });
    auto end = std::find_if_not(s.rbegin(), s.rend(), [](unsigned char ch) { return std::isspace(ch); }).base();
    return (start < end ? std::string(start, end) : std::string());
}
