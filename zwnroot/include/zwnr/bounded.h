#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>
#include <zwnr/region/node.h>
#include <zwnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

enum zwnr_bounded_damage {
  ZWNR_BOUNDED_DAMAGE_HALF_SIZE = 1 << 0,
  ZWNR_BOUNDED_DAMAGE_REGION = 1 << 1,
  ZWNR_BOUNDED_DAMAGE_TITLE = 1 << 2,
};

struct zwnr_bounded_move_event {
  uint32_t serial;
  struct zwnr_seat *seat;
  struct wl_client *client;
};

struct zwnr_bounded {
  struct zwnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal mapped;   // (NULL)
    struct wl_signal destroy;  // (NULL)
    struct wl_signal move;     // (struct zwnr_bounded_move_event*)
    struct wl_signal title;    // (NULL)
  } events;

  struct {
    vec3 half_size;
    struct zwnr_region_node *region;  // nullable
    char *title;                      // nonnull, null-terminated, can be empty

    /**
     * Bit sum of zwnr_bounded_damage
     * User can set 0 to each bit, zwnr only set 1 to each bit
     */
    uint32_t damage;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
