#pragma once

#include "Base.h"
#include "Engine/Utils/Log.hpp"

#ifdef DEBUG
    #define CORE_ASSERT(x, ...)                                                                                        \
        {                                                                                                              \
            if (!(x))                                                                                                  \
            {                                                                                                          \
                LOG_CORE_ERROR(__VA_ARGS__);                                                                                     \
                DEBUGBREAK();                                                                                          \
            }                                                                                                          \
        }
#else
    #define CORE_ASSERT(x, ...)
#endif
