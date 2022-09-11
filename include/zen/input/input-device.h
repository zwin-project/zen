#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

struct zn_input_device {
  struct wlr_input_device* wlr_input;

  struct zn_seat* seat;
  struct wl_list link;  // zn_seat::devices

  union {
    struct zn_keyboard* keyboard;
    struct zn_pointer* pointer;
  };

  struct wl_listener wlr_input_destroy_listener;
  struct wl_listener seat_destroy_listener;
};

struct zn_input_device* zn_input_device_create(
    struct zn_seat* seat, struct wlr_input_device* wlr_input);

void zn_input_device_destroy(struct zn_input_device* self);
