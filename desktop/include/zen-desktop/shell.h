#pragma once

#include <wayland-server-core.h>

struct zn_desktop_shell {
  struct wl_listener new_screen_listener;
};

struct zn_desktop_shell *zn_desktop_shell_create(void);

void zn_desktop_shell_destroy(struct zn_desktop_shell *self);
