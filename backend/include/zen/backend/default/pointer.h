#pragma once

#include <wayland-server-core.h>

#include "zen/backend/default/input-device.h"
#include "zen/backend/default/seat.h"

struct zn_seat;

struct zn_pointer {
  struct zn_input_device_base base;
  struct zn_default_backend_seat *seat;  // @nonnull, @outlive

  struct wl_listener motion_listener;
  struct wl_listener wlr_input_device_destroy_listener;
  struct wl_listener seat_destroy_listener;
};

struct zn_pointer *zn_pointer_get(struct zn_input_device_base *base);

struct zn_pointer *zn_pointer_create(struct zn_default_backend_seat *seat,
    struct wlr_input_device *wlr_input_device);

void zn_pointer_destroy(struct zn_pointer *self);
