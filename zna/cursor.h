#pragma once

#include "base-unit.h"
#include "zen/appearance/cursor.h"
#include "zen/renderer/virtual-object.h"

struct zna_cursor {
  struct zn_cursor *zn_cursor;  // nonnull
  struct zna_system *system;

  // null when the current session does not exist, not null otherwise
  struct znr_virtual_object *virtual_object;

  struct zna_base_unit *base_unit;

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};
