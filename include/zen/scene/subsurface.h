#pragma once

#include <wlr/types/wlr_surface.h>

#include "zen/scene/view-child.h"

struct zn_subsurface {
  struct zn_view_child base;
  struct wlr_subsurface *wlr_subsurface;

  struct wl_list link;  // zn_view::subsurface_list;

  struct wl_listener wlr_subsurface_destroy_listener;
};

void zn_subsurface_init_subsurfaces(
    struct zn_view_child *view_child, struct wlr_surface *surface);

struct zn_subsurface *zn_subsurface_create(struct zn_view *view,
    struct zn_view_child *parent, struct wlr_subsurface *wlr_subsurface);
