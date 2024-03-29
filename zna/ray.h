#pragma once

#include "base-unit.h"
#include "ray/origin-unit.h"
#include "ray/tip-unit.h"
#include "zen/appearance/ray.h"
#include "zen/renderer/virtual-object.h"

struct zna_ray {
  struct zn_ray *zn_ray;      // nonnull
  struct zna_system *system;  // nonnull

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object *virtual_object;

  struct zna_ray_origin_unit *origin_unit;  // nonnull
  struct zna_ray_tip_unit *tip_unit;        // nonnull

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};
