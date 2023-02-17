#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_pointer.h>

struct zn_cursor;

struct zn_seat {
  struct zn_cursor *cursor;  // @nonnull, @owning

  struct {
    struct wl_signal pointer_motion;  // (struct wlr_event_pointer_motion *)
  } events;
};

void zn_seat_notify_motion(
    struct zn_seat *self, struct wlr_event_pointer_motion *event);
