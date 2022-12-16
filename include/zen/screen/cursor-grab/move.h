#pragma once

#include <wayland-server-core.h>

#include "zen/cursor.h"

struct zn_view;
struct zn_board;

struct zn_move_cursor_grab {
  struct zn_cursor_grab base;
  struct zn_view *view;  // nonnull

  struct zn_board *init_board;
  double init_x, init_y;
  double diff_x, diff_y;

  struct wl_listener view_destroy_listener;
  struct wl_listener init_board_destroy_listener;
};

void zn_move_cursor_grab_start(struct zn_cursor *cursor, struct zn_view *view);
