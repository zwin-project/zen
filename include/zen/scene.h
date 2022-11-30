#pragma once

#include <wayland-server-core.h>

struct zn_scene {
  struct wl_list board_list;
};

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy(struct zn_scene *self);
