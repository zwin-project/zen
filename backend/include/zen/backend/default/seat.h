#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_seat.h>

#include "zen/backend/default/backend.h"
#include "zen/backend/seat.h"

struct zn_default_backend_seat {
  struct zn_seat base;

  struct wlr_seat *wlr_seat;  // @nonnull, @owning

  struct wl_list device_list;  // zn_input_device_base::link

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

void zn_default_backend_seat_notify_motion(struct zn_default_backend_seat *self,
    struct zn_seat_pointer_motion_event *event);

void zn_default_backend_seat_update_capabilities(
    struct zn_default_backend_seat *self);

void zn_default_backend_seat_handle_new_input(
    struct zn_default_backend_seat *self, struct wlr_input_device *device);

struct zn_default_backend_seat *zn_default_backend_seat_get(
    struct zn_seat *base);

struct zn_default_backend_seat *zn_default_backend_seat_create(
    struct wl_display *display, const char *seat_name);

void zn_default_backend_seat_destroy(struct zn_default_backend_seat *self);
