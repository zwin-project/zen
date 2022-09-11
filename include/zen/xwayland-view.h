#pragma once

#include <wlr/xwayland.h>

#include "zen/scene/view.h"
#include "zen/server.h"

/** this destroys itself when the given wlr_xwayland_surface is destroyed */
struct zn_xwayland_view {
  struct zn_view base;
  struct wlr_xwayland_surface *wlr_xwayland_surface;  // nonnull

  struct zn_server *server;

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener move_listener;
  struct wl_listener wlr_xwayland_surface_destroy_listener;
  struct wl_listener wlr_surface_commit_listener;
};

struct zn_xwayland_view *zn_xwayland_view_create(
    struct wlr_xwayland_surface *xwayland_surface, struct zn_server *server);
