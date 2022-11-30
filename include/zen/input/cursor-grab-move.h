#pragma once

#include "zen/cursor.h"
#include "zen/input/cursor-grab.h"
#include "zen/scene/view.h"

struct zn_cursor_grab_move {
  struct zn_cursor_grab base;
  struct zn_view* view;  // nonnull

  struct zn_board* init_board;
  double init_x, init_y;
  double diff_x, diff_y;

  struct wl_listener view_unmap_listener;
  struct wl_listener init_board_destroy_listener;
};

void zn_cursor_grab_move_start(struct zn_cursor* cursor, struct zn_view* view);
