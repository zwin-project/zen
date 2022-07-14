#ifndef ZEN_SCENE_INTERNAL_H
#define ZEN_SCENE_INTERNAL_H

#include <wayland-server.h>

struct zn_scene {
  struct wl_list output_list;  // zn_scene_output::link, used by zn_scene_output
};

#endif  //  ZEN_SCENE_INTERNAL_H
