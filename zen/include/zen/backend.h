#pragma once

#include <wayland-server-core.h>

struct zn_backend {
  struct {
    struct wl_signal new_screen;  // (struct zn_screen *)
  } events;
};
