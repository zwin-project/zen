#pragma once

#include "node.h"
#include "zen/board.h"

struct zns_board {
  struct zn_board *zn_board;  // (nonnull)

  struct zns_node *node;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener zn_board_destroy_listener;
};

float zns_board_ray_cast(
    struct zns_board *self, vec3 origin, vec3 direction, float *u, float *v);

struct zns_board *zns_board_create(struct zn_board *zn_board);
