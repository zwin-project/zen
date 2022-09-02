#ifndef ZEN_CURSOR_H
#define ZEN_CURSOR_H

#include <wayland-server.h>
#include <wlr/render/wlr_texture.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "zen/input/cursor-grab.h"
#include "zen/input/input-device.h"
#include "zen/input/seat.h"
#include "zen/scene/screen.h"

struct zn_cursor {
  double x, y;
  uint32_t width, height;
  int hotspot_x, hotspot_y;

  struct zn_screen* screen;     // nullable
  struct wlr_surface* surface;  // nullable
  // if surface is not NULL, this is the texture of that surface. 
  struct wlr_texture* texture;  // nullable
  struct wlr_xcursor_manager* xcursor_manager;

  struct zn_cursor_grab* grab;
  struct zn_cursor_grab grab_default;

  struct wl_listener new_screen_listener;
  struct wl_listener screen_destroy_listener;
  struct wl_listener surface_commit_listener;
  struct wl_listener surface_destroy_listener;
};

void zn_cursor_move_relative(struct zn_cursor* self, double dx, double dy);

void zn_cursor_get_fbox(struct zn_cursor* self, struct wlr_fbox* fbox);

void zn_cursor_set_surface(struct zn_cursor* self, struct wlr_surface* surface,
    int hotspot_x, int hotspot_y);

void zn_cursor_set_xcursor(struct zn_cursor* self, char* name);

struct zn_cursor* zn_cursor_create(void);

void zn_cursor_destroy(struct zn_cursor* self);

#endif  // ZEN_CURSOR_H
