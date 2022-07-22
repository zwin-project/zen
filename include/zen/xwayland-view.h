#ifndef ZEN_XWAYLAND_VIEW_H
#define ZEN_XWAYLAND_VIEW_H

#include <wlr/xwayland.h>

#include "zen/server.h"

/** this destroys itself when the given wlr_xwayland_surface is destroyed */
struct zn_xwayland_view;

struct zn_xwayland_view *zn_xwayland_view_create(
    struct wlr_xwayland_surface *xwayland_surface, struct zn_server *server);

#endif  //  ZEN_XWAYLAND_VIEW_H
