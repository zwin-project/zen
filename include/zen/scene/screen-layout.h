#ifndef ZEN_SCREEN_LAYOUT_H
#define ZEN_SCREEN_LAYOUT_H

#include <wayland-server-core.h>

#include "zen/scene/scene.h"

struct zn_screen;

struct zn_screen_layout {
  struct zn_scene* scene;

  struct wl_list screens;  // zn_screen::link

  struct {
    // Emitted when a zn_screen is added
    struct wl_signal new_screen;  // (struct zn_screen *)
  } events;
};

// Returns the screen at the given x, y of the coordinates of screen layout.
// And sets the screen-local coordinates to dst_{x,y}
struct zn_screen* zn_screen_layout_get_closest_screen(
    struct zn_screen_layout* self, int x, int y, int* dst_x, int* dst_y);

void zn_screen_layout_add(
    struct zn_screen_layout* self, struct zn_screen* screen);

void zn_screen_layout_remove(
    struct zn_screen_layout* self, struct zn_screen* screen);

struct zn_screen_layout* zn_screen_layout_create(struct zn_scene* scene);

void zn_screen_layout_destroy(struct zn_screen_layout* self);

#endif  //  ZEN_SCREEN_LAYOUT_H
