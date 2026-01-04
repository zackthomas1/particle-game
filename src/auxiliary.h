#pragma once
#include "raylib.h"

#define PASSERT(cond, level, format, ...) \
    do { \
        if (!(cond)) { \
            TraceLog(level, "Assertion Failed: " format, ##__VA_ARGS__); \
        } \
    } while (0)

#define PASSERTABORT(cond, level, format, ...) \
    do { \
        if (!(cond)) { \
            TraceLog(level, "Assertion Failed: " format, ##__VA_ARGS__); \
            abort(); \
        } \
    } while (0)

#define PASSERTRETURN(cond, level, format, ...)\
    do { \
        if(!(cond)) { \
            TraceLog(level, "Assertion Failed: " format, ##__VA_ARGS__); \
            return; \
        } \
    } while (0)

inline static float GetRandomValueF()
{
    return ((2.0f * ((float)GetRandomValue(0, INT32_MAX) / (float)INT32_MAX)) - 1.0f);
}