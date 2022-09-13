#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/scene/view.h"

struct zn_view_child;

struct zn_view_child_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view_child *child);
};

/*
  A view child is a surface in the view tree, such as a subsurface or a popup
*/
struct zn_view_child {
  const struct zn_view_child_impl *impl;

  struct zn_view *view;          // nullable
  struct zn_view_child *parent;  // nullable

  // struct wl_listener view_unmap_listener;
};

void zn_view_child_damage(struct zn_view_child *self);

void zn_view_child_damage_whole(struct zn_view_child *self);

void zn_view_child_init(struct zn_view_child *self, struct zn_view *view);
