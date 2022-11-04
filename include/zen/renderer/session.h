#pragma once

#include <wayland-server-core.h>

struct znr_session {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};
