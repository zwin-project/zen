#pragma once

#include <wayland-server-core.h>

struct zn_screen;
struct zn_ray;

struct zn_scene {
  struct wl_list screen_list;  // zn_screen::link;
  struct wl_list board_list;   // zn_board::link

  // TODO: struct zn_cursor* cursor;  // nonnull
  struct zn_ray *ray;  // nonnull

  struct {
    struct wl_signal new_board;  // (struct zn_board*)
  } events;
};

void zn_scene_new_screen(struct zn_scene *self, struct zn_screen *screen);

struct zn_scene *zn_scene_create(void);

void zn_scene_destroy_resources(struct zn_scene *self);

void zn_scene_destroy(struct zn_scene *self);
