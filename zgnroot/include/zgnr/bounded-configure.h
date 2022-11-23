#pragma once

#include <cglm/vec3.h>
#include <wayland-server-core.h>

struct zgnr_bounded_configure {
  vec3 half_size;
  struct wl_list link;  // zgnr_bounded_impl::configure_list
  uint32_t serial;
};
