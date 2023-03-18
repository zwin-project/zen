#pragma once

#include <wayland-server-core.h>

#include "input-device.h"

struct zn_seat;

struct zn_pointer {
  struct zn_input_device_base base;

  struct wl_listener motion_listener;
  struct wl_listener button_listener;
  struct wl_listener axis_listener;
  struct wl_listener frame_listener;
  struct wl_listener wlr_input_device_destroy_listener;
};

struct zn_pointer *zn_pointer_get(struct zn_input_device_base *base);

struct zn_pointer *zn_pointer_create(struct wlr_input_device *wlr_input_device);
