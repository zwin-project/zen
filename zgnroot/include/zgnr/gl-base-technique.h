#pragma once

#include <wayland-server-core.h>
#include <zgnr/rendering-unit.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_base_technique {
  struct zgnr_rendering_unit *unit;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
