#pragma once

#include <wayland-server-core.h>

struct zn_xr_compositor {
  // must be connected state
  struct zn_xr_system *xr_system;  // @nonnull, @outlive

  struct wl_listener xr_system_session_state_changed_listener;

  struct wl_global *global;  // @nonnull, @owning

  struct wl_list resource_list;  // wl_resource::link of zwn_compositor

  struct {
    struct wl_signal destroy;
  } events;
};

/// @param xr_system must be in connected state
struct zn_xr_compositor *zn_xr_compositor_create(
    struct wl_display *display, struct zn_xr_system *xr_system);

void zn_xr_compositor_destroy(struct zn_xr_compositor *self);
