#pragma once

#include <wlr/types/wlr_seat.h>

#include "zen/seat.h"

struct zn_seat *zn_seat_create(struct wl_display *display);

void zn_seat_destroy(struct zn_seat *self);

void zn_seat_notify_pointer_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event);

void zn_seat_notify_pointer_button(
    struct zn_seat *self, struct wlr_event_pointer_button *event);

void zn_seat_notify_pointer_axis(
    struct zn_seat *self, struct wlr_event_pointer_axis *event);

void zn_seat_notify_pointer_frame(struct zn_seat *self);

/// @param capabilities is a bitfield of enum wl_seat_capability
void zn_seat_notify_capabilities(struct zn_seat *self, uint32_t capabilities);
