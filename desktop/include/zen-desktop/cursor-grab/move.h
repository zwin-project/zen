#pragma once

#include <wayland-server-core.h>

#include "zen-desktop/cursor-grab.h"

struct zn_desktop_view;

struct zn_cursor_move_grab {
  struct zn_cursor_grab base;

  struct zn_desktop_view *view;  // @nonnull, @outlive

  /// the view position relative to the cursor position
  vec2 initial_view_cursor_position;

  struct wl_listener view_destroy_listener;
};

/// @returns true if successful, false otherwise
bool zn_cursor_move_grab_start(struct zn_desktop_view *view);
