#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>

struct zn_view;

struct zn_xdg_toplevel {
  struct wlr_xdg_toplevel *wlr_xdg_toplevel;  // nonnull

  struct zn_view *view;  // null if not mapped

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener wlr_xdg_surface_destroy_listener;
};

struct zn_xdg_toplevel *zn_xdg_toplevel_create(
    struct wlr_xdg_toplevel *toplevel);
