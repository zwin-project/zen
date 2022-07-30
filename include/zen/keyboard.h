#ifndef ZEN_KEYBOARD_H
#define ZEN_KEYBOARD_H

#include <wayland-server.h>

#include "zen/input-device.h"
#include "zen/seat.h"

struct zn_keyboard {
  struct wl_list link;  // zn_seat::keyboards
  struct wl_listener key_listener;
};

struct zn_keyboard* zn_keyboard_create(
    struct zn_seat* seat, struct wlr_input_device* input_device);

void zn_keyboard_destroy(struct zn_keyboard* self);

#endif  // ZEN_KEYBOARD_H
