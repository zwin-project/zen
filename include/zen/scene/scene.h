#ifndef ZEN_SCENE_H
#define ZEN_SCENE_H

#include <wayland-server-core.h>

struct zn_scene {
  struct wl_list screens;  // zn_screen::link
};

struct zn_scene* zn_scene_create();

void zn_scene_destroy(struct zn_scene* self);

#endif  //  ZEN_SCENE_H
