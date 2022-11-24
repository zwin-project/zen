#pragma once

#include "base-unit.h"
#include "zen/appearance/ray.h"
#include "zen/renderer/virtual-object.h"

struct zna_ray {
  struct zn_ray* zn_ray;      // nonnull
  struct zna_system* system;  // nonnull

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object* virtual_object;

  struct zna_base_unit* base_unit;  // nonnull

  struct wl_listener motion_listener;
  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};
