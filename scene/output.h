#ifndef ZEN_SCENE_OUTPUT_H
#define ZEN_SCENE_OUTPUT_H

#include <wayland-server.h>

#include "zen-scene.h"

struct zn_scene_output {
  struct wl_list link;  // zn_scene::output_list

  struct wlr_output *wlr_output;  // nonnull

  struct wl_list toplevels;  // zn_scene_toplevel_view::link
};

#endif  //  ZEN_SCENE_OUTPUT_H
