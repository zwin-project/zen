#pragma once

#include "input-device.h"

struct zn_keyboard {
  struct zn_input_device_base base;

  struct wl_listener wlr_input_device_destroy_listener;
  struct wl_listener keyboard_key_listener;
  struct wl_listener keyboard_modifiers_listener;
};

struct zn_keyboard *zn_keyboard_create(
    struct wlr_input_device *wlr_input_device);
