#ifndef ZEN_SCENE_TOPLEVEL_VIEW_H
#define ZEN_SCENE_TOPLEVEL_VIEW_H

#include <wayland-server-core.h>
#include <wlr/types/wlr_xdg_shell.h>

struct zn_scene_toplevel_view {
  struct wlr_xdg_toplevel* wlr_xdg_toplevel;

  struct wl_list link;  // zn_scene_output::toplevels
};

#endif  //  ZEN_SCENE_XDG_TOPLEVEL_VIEW_H
