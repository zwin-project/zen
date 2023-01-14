#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zwnr_bounded_configure {
  vec3 half_size;
  struct wl_list link;  // zwnr_bounded_impl::configure_list
  uint32_t serial;
};
