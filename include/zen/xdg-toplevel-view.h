#pragma once

#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/view.h"
#include "zen/server.h"

/** this destroys itself when the given wlr_xdg_surface is destroyed */
struct zn_xdg_toplevel_view {
  struct zn_view base;
  struct wlr_xdg_toplevel *wlr_xdg_toplevel;  // nonnull

  struct zn_server *server;

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener new_popup_listener;
  struct wl_listener move_listener;
  struct wl_listener resize_listener;
  struct wl_listener wlr_xdg_surface_ack_configure_listener;
  struct wl_listener wlr_xdg_surface_destroy_listener;
  struct wl_listener wlr_surface_commit_listener;
};

struct zn_xdg_toplevel_view *zn_xdg_toplevel_view_create(
    struct wlr_xdg_toplevel *xdg_toplevel, struct zn_server *server);
