#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>

struct zn_view_child;
struct zn_xdg_toplevel;

struct zn_xdg_popup {
  struct wlr_xdg_popup *wlr_xdg_popup;  // nonnull
  struct zn_view_child *view_child;

  struct zn_xdg_toplevel *toplevel;  // nonnull

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener new_popup_listener;
  struct wl_listener wlr_xdg_surface_destroy_listener;
};

struct zn_xdg_popup *zn_xdg_popup_create(
    struct wlr_xdg_popup *popup, struct zn_xdg_toplevel *toplevel);
