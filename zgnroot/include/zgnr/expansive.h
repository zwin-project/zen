#pragma once

#include <wayland-server-core.h>
#include <zgnr/region/node.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_expansive {
  struct zgnr_virtual_object *virtual_object;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct zgnr_region_node *region;  // nullable
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
