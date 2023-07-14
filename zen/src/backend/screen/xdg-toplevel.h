#pragma once

#include <wlr/types/wlr_xdg_shell.h>

struct zn_view;
struct zn_surface_snode;
struct zn_xdg_decoration;

struct zn_xdg_toplevel {
  struct wlr_xdg_toplevel *wlr_xdg_toplevel;  // @nonnull, @outlive

  struct zn_view *view;  // @nonnull, @owning

  struct zn_xdg_decoration *decoration;  // @nullable, @ref

  // @nonnull while mapped
  // Automatically destroyed when given wlr_surface is destroyed;
  struct zn_surface_snode *surface_snode;

  // Developer note: Reading through the wlroots source, the unmap signal is
  // always emitted before the destroy signal
  struct wl_listener surface_destroy_listener;
  struct wl_listener surface_map_listener;
  struct wl_listener surface_unmap_listener;
  struct wl_listener surface_commit_listener;  // listen only when mapped
  struct wl_listener surface_move_request_listener;
  struct wl_listener surface_resize_request_listener;
  struct wl_listener decoration_destroy_listener;
};

/// @param decoration may NULL
void zn_xdg_toplevel_set_decoration(
    struct zn_xdg_toplevel *self, struct zn_xdg_decoration *decoration);

/// @return value may NULL
struct zn_xdg_toplevel *zn_xdg_toplevel_from_wlr_xdg_surface(
    struct wlr_xdg_surface *surface);

struct zn_xdg_toplevel *zn_xdg_toplevel_create(
    struct wlr_xdg_toplevel *wlr_xdg_toplevel);
