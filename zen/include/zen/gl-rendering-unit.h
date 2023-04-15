#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_rendering_unit {
  void *impl_data;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

#ifdef __cplusplus
}
#endif
