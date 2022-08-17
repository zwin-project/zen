#ifndef ZEN_XDG_POPUP_VIEW_H
#define ZEN_XDG_POPUP_VIEW_H

#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/view.h"
#include "zen/server.h"

struct zn_xdg_popup_view {
  struct zn_view base;
  struct wlr_xdg_popup* wlr_xdg_popup;

  struct zn_server* server;
  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener wlr_xdg_surface_destroy_listener;
};

struct zn_xdg_popup_view* zn_xdg_popup_view_create(
    struct wlr_xdg_popup* wlr_xdg_popup, struct zn_server* server);

#endif  //  ZEN_XDG_POPUP_VIEW_H
