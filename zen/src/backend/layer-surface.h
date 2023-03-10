#pragma once

#include <wlr/types/wlr_layer_shell_v1.h>

struct zn_layer_surface {
  struct wlr_layer_surface_v1 *wlr_layer_surface;  // @nonnull, @outlive

  struct wl_listener surface_destroy_listener;
  struct wl_listener surface_map_listener;
  struct wl_listener surface_unmap_listener;
  struct wl_listener surface_commit_listener;
};

struct zn_layer_surface *zn_layer_surface_create(
    struct wlr_layer_surface_v1 *wlr_layer_surface);
