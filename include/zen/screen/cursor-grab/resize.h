#pragma once

#include <wayland-server-core.h>

#include "zen/cursor.h"

struct zn_view;

struct zn_resize_cursor_grab {
  struct zn_cursor_grab base;
  struct zn_view *view;

  double init_view_width, init_view_height;
  double init_cursor_x, init_cursor_y;

  struct wl_listener view_destroy_listener;
};

void zn_resize_cursor_grab_start(
    struct zn_cursor *cursor, struct zn_view *view, uint32_t edges);
