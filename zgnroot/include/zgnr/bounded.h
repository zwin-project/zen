#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zgnr/region/node.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zgnr_bounded_damage {
  ZGNR_BOUNDED_DAMAGE_HALF_SIZE = 1 << 0,
  ZGNR_BOUNDED_DAMAGE_REGION = 1 << 1,
  ZGNR_BOUNDED_DAMAGE_TITLE = 1 << 2,
};

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
    struct wl_signal title;    // (NULL)
  } events;

  struct {
    vec3 half_size;
    struct zgnr_region_node *region;  // nullable
    char *title;                      // nonnull, null-terminated, can be empty

    /**
     * Bit sum of zgnr_bounded_damage
     * User can set 0 to each bit, zgnr only set 1 to each bit
     */
    uint32_t damage;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
