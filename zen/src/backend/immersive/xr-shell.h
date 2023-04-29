#pragma once

#include <wayland-server-core.h>

struct zn_xr_shell {
  struct wl_global *global;  // @nonnull, @outlive
};

struct zn_xr_shell *zn_xr_shell_create(struct wl_display *display);

void zn_xr_shell_destroy(struct zn_xr_shell *self);
