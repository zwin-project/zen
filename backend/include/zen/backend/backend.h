#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>

#include "zen/backend.h"

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;  // @nonnull, @owning

  struct wl_listener new_output_listener;
};
