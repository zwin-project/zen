#pragma once

#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <wayland-server-core.h>

struct zwnr_region_node {
  struct wl_list cuboid_list;  // zwnr_cuboid_region::link
};

float zwnr_region_node_ray_cast(
    struct zwnr_region_node *self, mat4 transform, vec3 origin, vec3 direction);
