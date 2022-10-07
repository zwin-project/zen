#pragma once

#include <wayland-server-core.h>
#include <zen-desktop-protocol.h>
#include <znr/remote.h>

#include "zen/immersive/display-system.h"
#include "zen/immersive/remote/renderer.h"

struct zn_remote_display_system {
  struct zn_immersive_display_system base;

  struct wl_global* global;
  struct wl_list resources;

  enum zen_display_system_type type;

  struct zn_remote_immersive_renderer* renderer;  // non null

  struct znr_remote* remote;  // non null
};
