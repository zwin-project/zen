#pragma once

#include <wayland-server-core.h>

struct zn_snode;

struct zn_cursor_motion_event {
  uint32_t time_msec;
  double delta_x, delta_y;
  double unaccel_dx, unaccel_dy;
};

struct zn_cursor {
  struct zn_snode *snode;  // @nonnull, @owning

  struct wlr_xcursor_manager *xcursor_manager;  // @nonnull @owning

  // TODO(@Aki-7) hotspot, enable to change cursor texture
  struct wlr_texture *xcursor_texture;  // @nullable, @owning

  struct {
    struct wl_signal motion;  // (struct zn_cursor_motion_event *)
  } events;
};
