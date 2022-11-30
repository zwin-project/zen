#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_session {
  struct {
    struct wl_signal disconnected;  // (NULL)
    struct wl_signal frame;         // (NULL)
  } events;
};

void znr_session_destroy(struct znr_session *session);

#ifdef __cplusplus
}
#endif
