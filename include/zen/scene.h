#pragma once

#include <wayland-server-core.h>

struct zn_screen;

struct zn_scene {
  struct wl_list screen_list;  // zn_screen::link;
  struct wl_list board_list;   // zn_board::link
};

void zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen);

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy_resources(struct zn_scene *self);

void zn_scene_destroy(struct zn_scene *self);
