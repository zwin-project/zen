#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zgnr/region/node.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_bounded_move_event {
  uint32_t serial;
  struct zgnr_seat *seat;
  struct wl_client *client;
};

struct zgnr_bounded {
  struct zgnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal mapped;   // (NULL)
    struct wl_signal destroy;  // (NULL)
    struct wl_signal move;     // (struct zgnr_bounded_move_event*)
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
