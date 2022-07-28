#ifndef ZEN_SCREEN_LAYOUT_H
#define ZEN_SCREEN_LAYOUT_H

#include <wayland-server-core.h>

// if include screen.h, fail to compile
struct zn_screen;

struct zn_screen_layout {
  struct wl_list screens;  // zn_screen::link
};

void zn_screen_layout_add(
    struct zn_screen_layout* self, struct zn_screen* screen);

struct zn_screen_layout* zn_screen_layout_create(void);

void zn_screen_layout_destroy(struct zn_screen_layout* self);

#endif  //  ZEN_SCREEN_LAYOUT_H
