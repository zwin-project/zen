#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

struct zn_pointer {
  struct wl_listener button_listener;
  struct wl_listener motion_listener;
  struct wl_listener axis_listener;
  struct wl_listener frame_listener;
};

struct zn_pointer* zn_pointer_create(struct wlr_input_device* input_device);

void zn_pointer_destroy(struct zn_pointer* self);
