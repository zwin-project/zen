#ifndef ZEN_SCREEN_LAYOUT_H
#define ZEN_SCREEN_LAYOUT_H

#include <wayland-server-core.h>

// if include screen.h, fail to compile
struct zn_screen;

struct zn_screen_layout {
  struct wl_list screens;  // zn_screen::link

  struct {
    // Emitted when a zn_screen is added
    struct wl_signal new_screen;  // (struct zn_screen *)
  } events;
};

// Searches for the screen from **global** coordinates,
// and return the screen corresponding to the position.
// If there is no screen, return NULL.
struct zn_screen* zn_screen_layout_get_screen_at(
    struct zn_screen_layout* self, int x, int y);

void zn_screen_layout_add(
    struct zn_screen_layout* self, struct zn_screen* screen);

void zn_screen_layout_remove(
    struct zn_screen_layout* self, struct zn_screen* screen);

struct zn_screen_layout* zn_screen_layout_create(void);

void zn_screen_layout_destroy(struct zn_screen_layout* self);

#endif  //  ZEN_SCREEN_LAYOUT_H
