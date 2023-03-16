#pragma once

#include "zen-desktop/cursor-grab.h"

struct zn_desktop_view;

struct zn_cursor_resize_grab {
  struct zn_cursor_grab base;

  struct zn_desktop_view *view;  // @nonnull, @outlive

  uint32_t edges;  // a bitfield of enum wlr_edges
  vec2 initial_view_size;
  vec2 initial_cursor_position;

  struct wl_listener view_destroy_listener;
};

/// @param edges is a bitfield of enum wlr_edges
bool zn_cursor_resize_grab_start(struct zn_desktop_view *view, uint32_t edges);
