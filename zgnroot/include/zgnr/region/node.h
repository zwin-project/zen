#pragma once

#include <wayland-server-core.h>

struct zgnr_region_node {
  struct wl_list cuboid_list;  // zgnr_cuboid_region::link
};
