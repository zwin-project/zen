#pragma once

#include <wayland-server-core.h>

struct zn_xr_compositor {
  struct wl_global *global;  // @nonnull, @owning

  struct {
    struct wl_signal destroy;
  } events;
};

struct zn_xr_compositor *zn_xr_compositor_create(struct wl_display *display);

void zn_xr_compositor_destroy(struct zn_xr_compositor *self);
