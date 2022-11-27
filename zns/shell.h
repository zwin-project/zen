#pragma once

#include <zgnr/shell.h>

#include "ray-grab/default.h"
#include "zen/shell/shell.h"

struct zns_bounded;

struct zn_shell {
  struct zgnr_shell* zgnr_shell;

  struct wl_list bounded_list;  // zns_bounded::link

  struct zns_default_ray_grab default_grab;

  struct wl_listener new_bounded_listener;
};

/**
 * @param distance returns the distance to the intersection if intersected.
 * @return the intersected zns_bounded, NULL if not intersected
 */
struct zns_bounded* zn_shell_ray_cast(
    struct zn_shell* self, struct zn_ray* ray, float* distance);
