#pragma once

#include <cglm/types.h>

#define ZN_COLOR_VEC3_INIT(r, g, b)  \
  {                                  \
    r / 256.f, g / 256.f, b / 256.f, \
  }

#define ZN_COLOR_VEC3(r, g, b) (vec3) ZN_COLOR_VEC3_INIT(r, g, b)

#define ZN_NAVY_VEC3 ZN_COLOR_VEC3(17.f, 31.f, 77.f)

#define ZN_NAVY_VEC3_INIT ZN_COLOR_VEC3_INIT(17.f, 31.f, 77.f)

#define ZN_NAVY_HIGHLIGHTED_VEC3 ZN_COLOR_VEC3(30.f, 47.f, 104.f)

#define ZN_NAVY_HIGHLIGHTED_VEC3_INIT ZN_COLOR_VEC3_INIT(30.f, 47.f, 104.f)
