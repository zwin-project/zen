#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/scene/view.h"

struct zn_view_child;

struct zn_view_child_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view_child *child);
  void (*get_view_coords)(struct zn_view_child *child, int *sx, int *sy);
};

/*
  A view child is a surface in the view tree, such as a subsurface or a popup
*/
struct zn_view_child {
  const struct zn_view_child_impl *impl;

  struct zn_view *view;          // nullable
  struct zn_view_child *parent;  // nullable

  bool mapped;

  struct wl_listener view_unmap_listener;
  struct wl_listener view_destroy_listener;
};

void zn_view_child_damage(struct zn_view_child *self);

void zn_view_child_map(struct zn_view_child *self);

void zn_view_child_unmap(struct zn_view_child *self);

void zn_view_child_init(struct zn_view_child *self,
    const struct zn_view_child_impl *impl, struct zn_view *view);

void zn_view_child_fini(struct zn_view_child *self);
