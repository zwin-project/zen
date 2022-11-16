#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_program {
  struct {
    struct wl_signal destroy;
  } events;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
