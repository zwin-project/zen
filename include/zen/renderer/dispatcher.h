#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_dispatcher {
  struct {
    struct wl_signal frame;  // (NULL)
  } events;
};

void znr_dispatcher_destroy(struct znr_dispatcher *self);

#ifdef __cplusplus
}
#endif
