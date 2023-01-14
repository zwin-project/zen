#pragma once

#include <wayland-server-core.h>
#include <zwnr/region/node.h>
#include <zwnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_expansive {
  struct zwnr_virtual_object *virtual_object;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct zwnr_region_node *region;  // nullable
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
