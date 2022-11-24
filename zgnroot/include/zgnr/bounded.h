#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zgnr/region/node.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_bounded {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    vec3 half_size;
    struct zgnr_region_node *region;  // nullable
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
