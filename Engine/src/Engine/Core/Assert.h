#pragma once

#include "Base.h"
#include "Engine/Utils/ConsoleLog.h"

#ifdef DEBUG
    #define CORE_ASSERT(x, ...)                                                                                        \
        {                                                                                                              \
            if (!(x))                                                                                                  \
            {                                                                                                          \
                LOGE(__VA_ARGS__);                                                                                     \
                DEBUGBREAK();                                                                                          \
            }                                                                                                          \
        }
#else
    #define CORE_ASSERT(x, ...)
#endif
