#ifndef ZEN_SCENE_OUTPUT_H
#define ZEN_SCENE_OUTPUT_H

#include <wayland-server.h>

struct zn_scene_output {
  struct wl_list link;  // zn_scene::output_list
};

#endif  //  ZEN_SCENE_OUTPUT_H
