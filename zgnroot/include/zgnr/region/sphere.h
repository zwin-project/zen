#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zgnr_sphere_region {
  vec3 center;
  float radius;

  struct wl_list link;
};
