#pragma once

#include <wlr/util/box.h>

struct zn_scene;
struct zn_screen;

struct zn_screen_layout {
  struct zn_scene *scene;
  struct wl_list screens;  // zn_screen::link

  struct {
    struct wl_signal new_screen;  // (struct zn_screen*)
  } events;
};

void zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_screen *new_screen);

void zn_screen_layout_remove(
    struct zn_screen_layout *self, struct zn_screen *screen);

int zn_screen_layout_len(struct zn_screen_layout *self);

struct zn_screen_layout *zn_screen_layout_create(struct zn_scene *scene);

void zn_screen_layout_destroy(struct zn_screen_layout *self);
