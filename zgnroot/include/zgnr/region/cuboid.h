#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zgnr_cuboid_region {
  vec3 half_size;
  vec3 center;
  versor quaternion;

  struct wl_list link;
};
