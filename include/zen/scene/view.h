#ifndef ZEN_VIEW_H
#define ZEN_VIEW_H

#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/scene.h"

struct zn_view;

struct zn_view_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view *view);
  void (*set_activated)(struct zn_view *view, bool activate);
};

enum zn_view_type {
  ZN_VIEW_XDG_TOPLEVEL,
  ZN_VIEW_XWAYLAND,
};

struct zn_view {
  double x, y;

  enum zn_view_type type;

  const struct zn_view_impl *impl;

  struct wl_list link;  // zn_board::view_list;

  struct {
    struct wl_signal unmap;
  } events;
};

void zn_view_get_fbox(struct zn_view *self, struct wlr_fbox *fbox);

bool zn_view_is_mapped(struct zn_view *self);

void zn_view_map_to_scene(struct zn_view *self, struct zn_scene *scene);

void zn_view_unmap(struct zn_view *self);

void zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl);

void zn_view_fini(struct zn_view *self);

#endif  //  ZEN_XDG_VIEW_H
