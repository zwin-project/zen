#pragma once

#include <wlr/util/box.h>

struct zn_scene;
struct zn_screen;

struct zn_screen_layout {
  struct wl_list screen_list;  // zn_screen::link
};

// Returns the screen at the given x, y of the coordinates of screen layout.
// And sets the screen-local coordinates to dest_{x,y}
struct zn_screen *zn_screen_layout_get_closest_screen(
    struct zn_screen_layout *self, double x, double y, double *dest_x,
    double *dest_y);

void zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_screen *new_screen);

void zn_screen_layout_remove(
    struct zn_screen_layout *self, struct zn_screen *screen);

int zn_screen_layout_screen_count(struct zn_screen_layout *self);

struct zn_screen_layout *zn_screen_layout_create(void);

void zn_screen_layout_destroy(struct zn_screen_layout *self);
