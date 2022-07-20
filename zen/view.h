#ifndef ZEN_VIEW_H
#define ZEN_VIEW_H

#include <wayland-server.h>
#include <wlr/types/wlr_xdg_shell.h>

enum zn_view_type {
  ZN_VIEW_XDG_TOPLEVEL,
};

struct zn_view {
  enum zn_view_type type;
};

void zn_view_init(struct zn_view *self, enum zn_view_type type);

void zn_view_fini(struct zn_view *self);

#endif  //  ZEN_XDG_VIEW_H
