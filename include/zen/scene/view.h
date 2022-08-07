#ifndef ZEN_VIEW_H
#define ZEN_VIEW_H

#include <wayland-server.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/screen.h"

struct zn_view;

struct zn_view_impl {
  struct wlr_surface *(*get_wlr_surface)(struct zn_view *view);
};

enum zn_view_type {
  ZN_VIEW_XDG_TOPLEVEL,
  ZN_VIEW_XWAYLAND,
};

struct zn_view {
  int x, y;

  enum zn_view_type type;

  const struct zn_view_impl *impl;

  struct wl_list link;  // zn_screen::views;
};

void zn_view_get_box(struct zn_view *self, struct wlr_box *box);

bool zn_view_is_mapped(struct zn_view *self);

void zn_view_map_to_screen(struct zn_view *self, struct zn_screen *screen);

void zn_view_unmap(struct zn_view *self);

void zn_view_init(struct zn_view *self, enum zn_view_type type,
    const struct zn_view_impl *impl);

void zn_view_fini(struct zn_view *self);

#endif  //  ZEN_XDG_VIEW_H
