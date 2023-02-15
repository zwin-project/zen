#pragma once

#include <wayland-server-core.h>

struct zn_cursor_motion_event {
  uint32_t time_msec;
  double delta_x, delta_y;
  double unaccel_dx, unaccel_dy;
};

struct zn_cursor {
  struct {
    struct wl_signal motion;  // (struct zn_cursor_motion_event *)
  } events;
};
