#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/cursor.h"

struct zn_view;

struct zn_down_cursor_grab {
  struct zn_cursor_grab base;
  struct zn_view *view;  // nonnull

  struct wl_listener view_destroy_listener;
};

void zn_down_cursor_grab_start(struct zn_cursor *cursor, struct zn_view *view);
