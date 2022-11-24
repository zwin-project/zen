#pragma once

#include <cglm/vec3.h>
#include <wayland-server-core.h>

struct zgnr_cuboid_region {
  vec3 half_size;
  vec3 center;
  versor quaternion;

  struct wl_list link;
};

struct zgnr_region {
  struct wl_list cuboid_list;  // zgnr_cuboid_region::link
};

struct zgnr_region* zgnr_region_create(struct wl_client* client, uint32_t id);
