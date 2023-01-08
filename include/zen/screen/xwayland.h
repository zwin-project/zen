#pragma once

#include <wayland-server-core.h>
#include <wlr/xwayland.h>

struct zn_view;

struct zn_xwayland_view {
  struct wlr_xwayland_surface *xwayland_surface;  // nonnull

  struct zn_view *view;  // null if not mapped

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener move_listener;
  struct wl_listener resize_listener;
  struct wl_listener maximize_listener;
  struct wl_listener wlr_xwayland_surface_destroy_listener;
};

struct zn_xwayland_view *zn_xwayland_view_create(
    struct wlr_xwayland_surface *xwayland_surface);
