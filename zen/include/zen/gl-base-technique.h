#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_base_technique {
  void *impl_data;  // @nullable, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

#ifdef __cplusplus
}
#endif
