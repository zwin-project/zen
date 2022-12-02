#pragma once

#include "node.h"
#include "zen/board.h"

struct zns_board {
  struct zn_board *zn_board;  // (nonnull)

  struct zns_node *node;

  struct wl_listener zn_board_destroy_listener;
};

struct zns_board *zns_board_create(struct zn_board *zn_board);
