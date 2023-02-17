#pragma once

#include "zen/backend.h"

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;      // @nonnull, @owning
  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct wl_list input_device_list;  // zn_input_device_base::link

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
};

void zn_default_backend_update_capabilities(struct zn_default_backend *self);
