#include <string>

static std::string operator"" S(const char8_t* str, std::size_t) { return reinterpret_cast<const char*>(str); }
static const char* operator"" C(const char8_t* str, std::size_t) { return reinterpret_cast<const char*>(str); }

#if defined(__cpp_char8_t)
    #define U8(x) u8##x##C
    #define U8S(x) u8##x##S
#else
    #define U8(x) u8##x
    #define U8S(x) std::string(u8##x)
#endif
