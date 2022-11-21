#pragma once

#include <wayland-server-core.h>

#include "zen/appearance/scene/virtual-object.h"
#include "zen/renderer/virtual-object.h"

struct zna_virtual_object {
  struct zn_virtual_object* zn_virtual_object;  // nonnull
  struct zna_system* system;                    // nonnull

  /**
   * null before the initial commit or when the current session does not exists,
   * nonnull otherwise
   */
  struct znr_virtual_object* znr_virtual_object;

  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
  struct wl_listener session_frame_listener;
  struct wl_listener commit_listener;
};
