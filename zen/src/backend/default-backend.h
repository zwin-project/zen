#pragma once

#include "zen/backend.h"

struct zn_compositor;
struct zn_view;
struct zn_xr;

struct zn_default_backend {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  /// It's null after zn_backend stops, nonnull otherwise
  struct wlr_backend *wlr_backend;  // @nullable, @owning

  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct zn_compositor *compositor;  // @nonnull, @owning

  // Basically owned by zn_default_backend, but can be destroyed when given
  // xr_system is destroyed.
  struct zn_xr_compositor *xr_compositor;  // @nullable

  struct zn_xr *xr;  // @nonnull, @owing

  struct wl_list input_device_list;  // zn_input_device_base::link

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
  struct wl_listener new_xr_system_listener;
  struct wl_listener xr_compositor_destroy_listener;
};

struct zn_default_backend *zn_default_backend_get(struct zn_backend *base);

void zn_default_backend_update_capabilities(struct zn_default_backend *self);

void zn_default_backend_notify_view_mapped(
    struct zn_default_backend *self, struct zn_view *view);
