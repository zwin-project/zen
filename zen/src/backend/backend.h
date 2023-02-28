#pragma once

#include "zen/backend.h"

struct zn_compositor;
struct zn_view;

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  /// It's null after zn_backend stops, nonnull otherwise
  struct wlr_backend *wlr_backend;  // @nullable, @owning

  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct zn_compositor *compositor;  // @nonnull, @owning

  struct wl_list input_device_list;  // zn_input_device_base::link

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
};

struct zn_default_backend *zn_default_backend_get(struct zn_backend *base);

void zn_default_backend_update_capabilities(struct zn_default_backend *self);

void zn_default_backend_notify_view_mapped(
    struct zn_default_backend *self, struct zn_view *view);