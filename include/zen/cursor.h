#ifndef ZEN_CURSOR_H
#define ZEN_CURSOR_H

#include <wayland-server.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "zen/input-device.h"
#include "zen/scene/screen.h"
#include "zen/seat.h"

struct zn_cursor {
  int x, y;
  int hotspot_x, hotspot_y;
  bool visible;
  struct zn_screen* screen;  // nullable
  struct wlr_texture* texture;
  struct wlr_surface* surface;
  struct wlr_xcursor_manager* xcursor_manager;

  struct wl_listener new_screen_listener;
  struct wl_listener destroy_screen_listener;
  struct wl_listener destroy_surface_listener;
};

void zn_cursor_move_relative(struct zn_cursor* self, int dx, int dy);

void zn_cursor_set_surface(struct zn_cursor* self, struct wlr_surface* surface,
    int hotspot_x, int hotspot_y);

void zn_cursor_reset_surface(struct zn_cursor* self);

struct zn_cursor* zn_cursor_create(void);

void zn_cursor_destroy(struct zn_cursor* self);

#endif  // ZEN_CURSOR_H
