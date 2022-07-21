#ifndef ZEN_INPUT_DEVICE_H
#define ZEN_INPUT_DEVICE_H

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

struct zn_input_device {
  struct wlr_input_device* wlr_input;
  struct wl_list link;  // zn_seat::devices
  struct zn_seat* seat;
  struct wl_listener device_destroy;
};

struct zn_input_device* zn_input_device_create(
    struct zn_seat* seat, struct wlr_input_device* wlr_input);

void zn_input_device_destroy(struct zn_input_device* self);

#endif  // ZEN_INPUT_DEVICE_H
