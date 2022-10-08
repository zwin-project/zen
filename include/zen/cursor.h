#pragma once

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
  bool visible;

  struct zn_screen* screen;  // nullable

  struct wlr_surface* surface;          // nullable
  const char* xcursor_name;             // nullable
  struct wlr_texture* xcursor_texture;  // nullable
  struct wlr_xcursor_manager* xcursor_manager;

  /**
   * While a **non-default** grab is in use, zn_cursor::screen will not be NULL
   */
  struct zn_cursor_grab* grab;
  struct zn_cursor_grab grab_default;

  struct wl_listener new_screen_listener;
  struct wl_listener screen_destroy_listener;
  struct wl_listener surface_commit_listener;
  struct wl_listener surface_destroy_listener;
};

void zn_cursor_start_grab(struct zn_cursor* self, struct zn_cursor_grab* grab);

void zn_cursor_end_grab(struct zn_cursor* self);

const char* zn_cursor_get_resize_xcursor_name(uint32_t edges);

void zn_cursor_move_relative(struct zn_cursor* self, double dx, double dy);

void zn_cursor_get_fbox(struct zn_cursor* self, struct wlr_fbox* fbox);

void zn_cursor_set_surface(struct zn_cursor* self, struct wlr_surface* surface,
    int hotspot_x, int hotspot_y);

void zn_cursor_set_xcursor(struct zn_cursor* self, const char* name);

struct zn_cursor* zn_cursor_create(void);

void zn_cursor_destroy_resources(struct zn_cursor* self);

void zn_cursor_destroy(struct zn_cursor* self);
