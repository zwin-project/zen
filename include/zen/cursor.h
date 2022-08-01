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
  struct zn_screen* screen;
  struct wlr_texture* texture;
  struct wlr_xcursor_manager* xcursor_manager;

  struct wl_listener add_screen_signal;
};

void zn_cursor_set_image(struct zn_cursor* self, const char* name);

struct zn_cursor* zn_cursor_create(void);

void zn_cursor_destroy(struct zn_cursor* self);

#endif  // ZEN_CURSOR_H
