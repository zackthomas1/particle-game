#pragma once
#include "raylib.h"

#define PASSERT(cond, level, fmsg, ...) if(!(cond)) {TraceLog(level, "Assertion Failed: %s.", fmsg); return; }

inline static float GetRandomValueF()
{
    return ((2.0f * ((float)GetRandomValue(0, INT32_MAX) / (float)INT32_MAX)) - 1.0f);
}