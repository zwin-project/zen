#pragma once

#include <wayland-server-core.h>

#include "input-device.h"

struct zn_default_backend;
struct zn_seat;

struct zn_pointer {
  struct zn_input_device_base base;

  struct zn_default_backend *backend;  // @nonnull, @outlive

  struct wl_listener motion_listener;
  struct wl_listener wlr_input_device_destroy_listener;
  struct wl_listener backend_destroy_listener;
};

struct zn_pointer *zn_pointer_get(struct zn_input_device_base *base);

struct zn_pointer *zn_pointer_create(struct zn_default_backend *backend,
    struct wlr_input_device *wlr_input_device);

void zn_pointer_destroy(struct zn_pointer *self);
