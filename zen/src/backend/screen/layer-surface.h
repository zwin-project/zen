#pragma once

#include <wlr/types/wlr_layer_shell_v1.h>

struct zn_surface_snode;
struct zn_output;

extern const struct zn_snode_interface zn_layer_surface_snode_implementation;

struct zn_layer_surface {
  struct wlr_layer_surface_v1 *wlr_layer_surface;  // @nonnull, @outlive

  struct zn_snode *snode;                  // @nonnull, @owning
  struct zn_surface_snode *surface_snode;  // @nonnull, @outlive

  struct zn_output *output;  // @nonnull, @outlive

  enum zwlr_layer_shell_v1_layer layer;

  struct wl_listener surface_destroy_listener;
  struct wl_listener surface_map_listener;
  struct wl_listener surface_unmap_listener;
  struct wl_listener surface_commit_listener;
  struct wl_listener output_destroy_listener;
};

/// @param output is nonnull
struct zn_layer_surface *zn_layer_surface_create(
    struct wlr_layer_surface_v1 *wlr_layer_surface, struct zn_output *output);
