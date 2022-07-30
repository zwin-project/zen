#ifndef ZEN_SEAT_H
#define ZEN_SEAT_H

#include <wayland-server.h>

#include "zen/keyboard.h"

#define ZEN_DEFAULT_SEAT "seat0"

struct zn_seat {
  struct wlr_seat* wlr_seat;
  struct wl_list devices;    // zn_input_device::link
  struct wl_list keyboards;  // zn_keyboard::link
};

void zn_seat_add_device(
    struct zn_seat* self, struct zn_input_device* input_device);

void zn_seat_remove_device(
    struct zn_seat* self, struct zn_input_device* input_device);

struct zn_seat* zn_seat_create(
    struct wl_display* display, const char* seat_name);

void zn_seat_destroy(struct zn_seat* self);

#endif  // ZEN_SEAT_H
