#ifndef ZEN_KEYBOARD_H
#define ZEN_KEYBOARD_H

#include <wayland-server.h>

#include "zen/input-device.h"

struct zn_keyboard {
  struct wl_listener key_listener;
};

struct zn_keyboard* zn_keyboard_create();

void zn_keyboard_destroy(struct zn_keyboard* self);

#endif  // ZEN_KEYBOARD_H
