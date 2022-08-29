#ifndef ZEN_VIEW_CHILD_H
#define ZEN_VIEW_CHILD_H

#include <wlr/types/wlr_surface.h>

#include "zen/scene/scene.h"

struct zn_view_child;

struct zn_view_child_impl {
  void (*get_view_coords)(struct zn_view_child *child, int *sx, int *sy);
};

struct zn_view_child {
  struct wlr_surface *surface;  // nonnull
  struct wl_list link;          // zn_view_child::child_list;

  const struct zn_view_child_impl *impl;

  struct zn_view *view;
  struct zn_view_child *parent;  // nullable
  struct wl_list *child_list;

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener wlr_surface_commit_listener;
  struct wl_listener wlr_surface_destroy_listener;
  struct wl_listener view_unmap_listener;
};

void zn_view_child_init(struct zn_view_child *self);

#endif  //  ZEN_VIEW_CHILD_H
