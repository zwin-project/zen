#pragma once

#include <wayland-server-core.h>

struct zn_xr_system_manager;
struct zn_xr_compositor;
struct zn_xr_shell;

struct zn_xr {
  struct zn_xr_system_manager *xr_system_manager;  // @nonnull, @owning

  struct zn_xr_compositor *xr_compositor;  // @nonnull, @owning
  struct zn_xr_shell *xr_shell;            // @nonnull, @owning
};

struct zn_xr *zn_xr_create(struct wl_display *display);

void zn_xr_destroy(struct zn_xr *self);
