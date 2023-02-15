#pragma once

#include <wayland-server-core.h>

struct zn_seat_pointer_motion_event {
  uint32_t time_msec;
  double delta_x, delta_y;
  double unaccel_dx, unaccel_dy;
};

struct zn_seat {
  struct {
    struct wl_signal pointer_motion;  // (struct zn_seat_pointer_motion_event *)
  } events;
};
