#pragma once

#include <wayland-server-core.h>

#include "zen/immersive/renderer.h"

struct zn_immersive_display_system {
  struct {
    struct wl_signal activate;
    struct wl_signal deactivated;
  } events;
};

struct zn_immersive_display_system* zn_immersive_display_system_create(
    struct wl_display* display, struct zn_immersive_renderer* renderer);

void zn_immersive_display_system_destroy(
    struct zn_immersive_display_system* self);
