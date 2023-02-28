#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>

struct zn_cursor;

struct zn_seat {
  struct zn_cursor *cursor;  // @nonnull, @owning

  struct {
    struct wl_signal pointer_motion;  // (struct wlr_event_pointer_motion *)
    struct wl_signal pointer_button;  // (struct wlr_event_pointer_button)
    struct wl_signal pointer_axis;    // (struct wlr_event_pointer_axis)
    struct wl_signal pointer_frame;   // (NULL)
  } events;
};

void zn_seat_notify_pointer_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event);

void zn_seat_notify_pointer_button(
    struct zn_seat *self, struct wlr_event_pointer_button *event);

void zn_seat_notify_pointer_axis(
    struct zn_seat *self, struct wlr_event_pointer_axis *event);

void zn_seat_notify_pointer_frame(struct zn_seat *self);
