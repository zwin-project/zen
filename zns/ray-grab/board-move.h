#pragma once

#include "zen/ray.h"

struct zns_board;

struct zns_board_move_ray_grab {
  struct zn_ray_grab base;

  struct zns_board *zns_board;  // nonnull

  vec3 local_tip;

  struct wl_listener zns_board_destroy_listener;
};

struct zns_board_move_ray_grab *zns_board_move_ray_grab_create(
    struct zns_board *zns_board);
