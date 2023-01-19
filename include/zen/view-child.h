#pragma once

#include <wlr/types/wlr_surface.h>

struct zn_view_child;

struct zn_view_child_interface {
  void (*get_toplevel_coords)(struct zn_view_child *child, double child_sx,
      double child_sy, double *toplevel_sx, double *toplevel_sy);
};

struct zn_view_child {
  void *user_data;
  struct wlr_surface *surface;  // nonnull

  const struct zn_view_child_interface *impl;

  struct zn_view *view;

  struct wl_listener commit_listener;
};

struct zn_view_child *zn_view_child_create(struct wlr_surface *surface,
    struct zn_view *view, const struct zn_view_child_interface *impl,
    void *user_data);

void zn_view_child_destroy(struct zn_view_child *self);
