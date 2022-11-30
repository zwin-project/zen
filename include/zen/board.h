#pragma once

#include <wayland-server-core.h>

struct zn_screen;

struct zn_board {
  struct wl_list link;  // zn_scene::board_list

  struct zn_screen *screen;  // nullable

  struct wl_listener screen_destroy_listener;

  float color[3];  // FIXME: debugging purpose, remove me later
};

bool zn_board_is_dangling(struct zn_board *self);

/**
 * @param screen is nullable
 */
void zn_board_set_screen(struct zn_board *self, struct zn_screen *screen);

struct zn_board *zn_board_create(void);

void zn_board_destroy(struct zn_board *self);
