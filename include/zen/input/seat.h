#pragma once

#include <wayland-server.h>

#include "zen/cursor.h"
#include "zen/ray.h"

#define ZEN_DEFAULT_SEAT "seat0"

struct zn_seat {
  struct wlr_seat* wlr_seat;
  struct wl_list devices;  // zn_input_device::link

  struct zn_cursor* cursor;  // nonnull
  struct zn_ray* ray;        // nonnull

  struct {
    struct wl_signal destroy;
  } events;

  struct wl_listener request_set_cursor_listener;
};

void zn_seat_add_device(
    struct zn_seat* self, struct zn_input_device* input_device);

void zn_seat_remove_device(
    struct zn_seat* self, struct zn_input_device* input_device);

struct zn_seat* zn_seat_create(
    struct wl_display* display, const char* seat_name);

void zn_seat_destroy(struct zn_seat* self);
