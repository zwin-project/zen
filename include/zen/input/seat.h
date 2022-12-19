#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_seat.h>
#include <zgnr/seat.h>

#define ZEN_DEFAULT_SEAT "seat0"

struct zn_input_device;

struct zn_seat {
  struct wlr_seat *wlr_seat;
  struct zgnr_seat *zgnr_seat;
  struct wl_list devices;  // zn_input_device::link

  struct wl_listener request_set_cursor_listener;

  uint32_t pressing_button_count;

  struct {
    struct wl_signal destroy;
  } events;
};

void zn_seat_add_device(
    struct zn_seat *self, struct zn_input_device *input_device);

void zn_seat_remove_device(
    struct zn_seat *self, struct zn_input_device *input_device);

struct zn_seat *zn_seat_create(
    struct wl_display *display, const char *seat_name);

void zn_seat_destroy(struct zn_seat *self);
