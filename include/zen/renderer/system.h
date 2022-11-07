#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_session;

struct znr_system {
  struct {
    // When the current session switches to a new one, current_session_destroyed
    // is called first, then current_session_created. That is, the following two
    // signals are always alternately emitted.
    struct wl_signal current_session_destroyed;  // (NULL)
    struct wl_signal current_session_created;    // (NULL)
  } events;

  struct znr_session* current_session;  // nullable
};

#ifdef __cplusplus
}
#endif
