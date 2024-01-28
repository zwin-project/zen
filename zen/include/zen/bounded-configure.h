#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_bounded_configure {
  uint32_t serial;
  vec3 half_size;
  struct wl_list link;  // zn_bounded::configure_list
};
