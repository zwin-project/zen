#pragma once

#include <zgnr/bounded.h>

#include "zen/ray.h"

struct zns_bounded {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct zgnr_bounded *zgnr_bounded;  // nonnull

  struct wl_listener zgnr_bounded_destroy_listener;
  struct wl_listener move_listener;

  struct wl_list link;  // zn_shell::bounded_list
};

/**
 * @returns the distance to the intersection, FLT_MAX if not intersected
 */
float zns_bounded_ray_cast(struct zns_bounded *self, struct zn_ray *ray);

struct zns_bounded *zns_bounded_create(struct zgnr_bounded *zgnr_bounded);
