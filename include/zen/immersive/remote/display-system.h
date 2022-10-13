#pragma once

#include <wayland-server-core.h>
#include <zen-desktop-protocol.h>
#include <znr/remote.h>

#include "zen/immersive/display-system.h"
#include "zen/immersive/remote/renderer.h"

struct zn_remote_immersive_display_system {
  struct zn_immersive_display_system base;

  struct wl_global* global;
  struct wl_list resources;

  enum zen_display_system_type type;

  struct zn_remote_immersive_renderer* renderer;  // non null

  struct znr_remote* remote;  // non null
};

struct zn_immersive_display_system* zn_remote_immersive_display_system_create(
    struct wl_display* display, struct zn_remote_immersive_renderer* renderer,
    struct znr_remote* remote);

void zn_remote_immersive_display_system_destroy(
    struct zn_immersive_display_system* parent);
