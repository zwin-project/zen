#pragma once

#include <wayland-server-core.h>

struct zn_immersive_display_system {
  struct {
    struct wl_signal activate;
    struct wl_signal deactivated;
  } events;
};
