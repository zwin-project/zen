#pragma once

#include <wlr/xwayland.h>

struct zn_view;

struct zn_xwayland_surface {
  struct wlr_xwayland_surface *wlr_xsurface;  // @nonnull, @outlive

  struct zn_view *view;  // @nonnull, @owning

  // Developer note: Reading through the wlroots source, the unmap signal is
  // emitted before the destroy signal
  struct wl_listener surface_destroy_listener;
  struct wl_listener surface_map_listener;
  struct wl_listener surface_unmap_listener;
};

struct zn_xwayland_surface *zn_xwayland_surface_create(
    struct wlr_xwayland_surface *wlr_xsurface);
