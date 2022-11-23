#pragma once

#include <cglm/vec3.h>
#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_bounded {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    vec3 half_size;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
