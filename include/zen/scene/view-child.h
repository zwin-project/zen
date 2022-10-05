#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/scene/view.h"

struct zn_view_child;

struct zn_view_child_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view_child *child);
  void (*get_toplevel_coords)(struct zn_view_child *child, double popup_sx,
      double popup_sy, double *toplevel_sx, double *toplevel_sy);
};

/*
  A view child is a surface in the view tree, such as a subsurface or a popup
*/
struct zn_view_child {
  const struct zn_view_child_impl *impl;

  struct zn_view *view;
  struct zn_view_child *parent;  // nullable

  bool mapped;
};

void zn_view_child_damage(struct zn_view_child *self);

void zn_view_child_map(struct zn_view_child *self);

void zn_view_child_unmap(struct zn_view_child *self);

void zn_view_child_init(struct zn_view_child *self,
    const struct zn_view_child_impl *impl, struct zn_view *view);
