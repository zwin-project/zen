#pragma once

#include <wayland-server.h>

struct zn_xr_gl {
  struct wl_global *global;
};

struct zn_xr_gl *zn_xr_gl_create(struct wl_display *display);

void zn_xr_gl_destroy(struct zn_xr_gl *self);
