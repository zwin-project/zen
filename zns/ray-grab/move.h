#pragma once

#include "bounded.h"
#include "zen/ray.h"

struct zns_move_ray_grab {
  struct zn_ray_grab base;

  vec3 local_tip;  // tip position in virtual local coordinates system
  struct zns_bounded* bounded;  // nonnull

  struct wl_listener bounded_destroy_listener;
};

/**
 * @param bounded is nonnull
 */
struct zns_move_ray_grab* zns_move_ray_grab_create(struct zns_bounded* bounded);
