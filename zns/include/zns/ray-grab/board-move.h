#pragma once

#include "zen/ray.h"

struct zns_board;

struct zns_board_move_ray_grab {
  struct zn_ray_grab base;

  struct zns_board *zns_board;  // nonnull

  vec3 local_tip;

  struct wl_listener zns_board_destroy_listener;
};

/**
 * @param zns_board is nonnull
 */
void zns_board_move_ray_grab_start(
    struct zn_ray *ray, struct zns_board *zns_board);
