#ifndef ZEN_XDG_POPUP_H
#define ZEN_XDG_POPUP_H

#include <wlr/types/wlr_xdg_shell.h>

#include "zen/scene/view-child.h"

struct zn_xdg_popup {
  struct wl_listener new_popup_listener;
  struct wl_listener view_child_destroy_listener;
};

struct zn_xdg_popup* zn_xdg_popup_create(struct wlr_xdg_popup* wlr_xdg_popup);

#endif  //  ZEN_XDG_POPUP_H
