#pragma once

#include <wayland-server-core.h>

#include "zen/appearance/scene/virtual-object.h"
#include "zen/renderer/virtual-object.h"

struct zna_virtual_object {
  struct zn_virtual_object* virtual_object;  // nonnull
  struct zna_system* system;                 // nonnull

  // null when current session does not exist
  struct znr_virtual_object* znr_virtual_object;

  struct wl_listener session_listener;
};
