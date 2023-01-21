#pragma once

#include "base-unit.h"
#include "zen/appearance/view-child.h"
#include "zen/renderer/virtual-object.h"

struct zna_view_child {
  struct zn_view_child *zn_view_child;  // nonnull
  struct zna_system *system;

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object *virtual_object;

  struct zna_base_unit *base_unit;

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};
