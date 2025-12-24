#include "pch.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#if defined(PLATFORM_DESKTOP)
    #if defined(GRAPHICS_API_OPENGL_43)
        #define GLSL_VERSION            430
    #elif defined(GRAPHICS_API_OPENGL_33)
        #define GLSL_VERSION            330
    #endif
#else   // PLATFORM_ANDROID, PLATFORM_WEB
    #define GLSL_VERSION            100
#endif