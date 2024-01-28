#pragma once

#include <wlr/types/wlr_surface.h>

struct zn_snode;

// Lifetime is identical with given wlr_surface
struct zn_surface_snode {
  struct zn_snode *snode;                   // @nonnull, @owning
  struct zn_snode *surface_node;            // @nonnull, @owning
  struct zn_snode *subsurfaces_below_node;  // @nonnull, @owning
  struct zn_snode *subsurfaces_above_node;  // @nonnull, @owning

  struct wlr_surface *surface;  // @nonnull, @outlive

  struct wl_listener new_subsurface_listener;
  struct wl_listener surface_commit_listener;
  struct wl_listener surface_destroy_listener;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

void zn_surface_snode_commit_damage(struct zn_surface_snode *self);

void zn_surface_snode_damage_whole(struct zn_surface_snode *self);

struct zn_surface_snode *zn_surface_snode_create(struct wlr_surface *surface);
