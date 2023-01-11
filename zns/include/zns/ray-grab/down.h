#pragma once

#include "zen/ray.h"

struct zns_node;

struct zns_down_ray_grab {
  struct zn_ray_grab base;

  struct zns_node *node;  // nonnull

  struct wl_listener node_destroy_listener;
};

void zns_down_ray_grab_start(struct zn_ray *ray, struct zns_node *node);
