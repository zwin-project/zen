#pragma once

#include <wayland-server-core.h>

struct zn_desktop_shell {
  struct zn_screen_layout *screen_layout;  // @nonnull, @owning

  struct zn_cursor_grab *cursor_grab;  // @nonnull, @owning

  struct wl_listener new_screen_listener;
  struct wl_listener pointer_motion_listener;
  struct wl_listener view_mapped_listener;
};

struct zn_desktop_shell *zn_desktop_shell_get_singleton(void);

struct zn_desktop_shell *zn_desktop_shell_create(void);

void zn_desktop_shell_destroy(struct zn_desktop_shell *self);