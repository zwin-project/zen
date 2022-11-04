#pragma once

#include <wayland-server-core.h>

struct znr_system {
  struct {
    struct wl_signal new_session;  // (struct znr_session*)
  } events;
};
