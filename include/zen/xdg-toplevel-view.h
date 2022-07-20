#ifndef ZEN_XDG_TOPLEVEL_VIEW_H
#define ZEN_XDG_TOPLEVEL_VIEW_H

#include <wlr/types/wlr_xdg_shell.h>

#include "zen/server.h"

/** this destroys itself when the given wlr_xdg_surface is destroyed */
struct zn_xdg_toplevel_view;

struct zn_xdg_toplevel_view *zn_xdg_toplevel_view_create(
    struct wlr_xdg_toplevel *xdg_toplevel, struct zn_server *server);

#endif  //  ZEN_XDG_TOPLEVEL_VIEW_H
