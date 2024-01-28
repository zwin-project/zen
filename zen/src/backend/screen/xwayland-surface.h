#pragma once

#include <wlr/xwayland.h>

struct zn_view;
struct zn_surface_snode;

struct zn_xwayland_surface {
  struct wlr_xwayland_surface *wlr_xsurface;  // @nonnull, @outlive

  struct zn_view *view;  // @nonnull, @owning

  // @nonnull while mapped
  // Automatically destroyed when given wlr_surface is destroyed.
  struct zn_surface_snode *surface_snode;

  // Developer note: Reading through the wlroots source, the unmap signal is
  // emitted before the destroy signal
  struct wl_listener surface_destroy_listener;
  struct wl_listener surface_map_listener;
  struct wl_listener surface_unmap_listener;
  struct wl_listener surface_configure_listener;
  struct wl_listener surface_move_request_listener;
  struct wl_listener surface_resize_request_listener;
  struct wl_listener surface_set_decoration_listener;
  struct wl_listener surface_commit_listener;  // listen only when mapped
  struct wl_listener snode_position_changed_listener;
};

struct zn_xwayland_surface *zn_xwayland_surface_create(
    struct wlr_xwayland_surface *wlr_xsurface);
