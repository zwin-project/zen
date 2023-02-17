#pragma once

#include <wayland-server-core.h>

struct zn_desktop_shell {
  struct zn_screen_layout *screen_layout;  // @nonnull, @owning

  struct wl_listener new_screen_listener;
};

struct zn_desktop_shell *zn_desktop_shell_get_singleton(void);

struct zn_desktop_shell *zn_desktop_shell_create(void);

void zn_desktop_shell_destroy(struct zn_desktop_shell *self);
