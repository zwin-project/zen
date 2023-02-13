#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>

#include "zen/backend.h"

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;      // @nonnull, @owning
  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct wl_listener new_output_listener;
};
