#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_session;

struct znr_system {
  struct {
    struct wl_signal current_session_changed;  // NULL
  } events;

  struct znr_session* current_session;  // nullable
};

#ifdef __cplusplus
}
#endif
