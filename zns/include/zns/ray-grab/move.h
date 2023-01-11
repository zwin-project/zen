#pragma once

#include "zen/ray.h"
#include "zns/bounded.h"

struct zns_move_ray_grab {
  struct zn_ray_grab base;

  vec3 local_tip;  // tip position in virtual local coordinates system
  struct zns_bounded *bounded;  // nonnull

  struct wl_listener bounded_destroy_listener;
};

/**
 * @param bounded is nonnull
 */
void zns_move_ray_grab_start(struct zn_ray *ray, struct zns_bounded *bounded);
