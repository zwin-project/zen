#ifndef ZEN_POINTER_H
#define ZEN_POINTER_H

#include <wayland-server.h>
#include <wlr/interfaces/wlr_pointer.h>

#include "zen/input/pointer_grab.h"

struct zn_pointer {
  struct zn_pointer_grab* grab;
  struct zn_pointer_grab grab_default;

  struct wl_listener button_listener;
  struct wl_listener motion_listener;
  struct wl_listener axis_listener;
  struct wl_listener frame_listener;
};

struct zn_pointer* zn_pointer_create(struct wlr_input_device* input_device);

void zn_pointer_destroy(struct zn_pointer* self);

#endif  // ZEN_POINTER_H
