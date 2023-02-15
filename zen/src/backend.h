#pragma once

#include <wayland-server-core.h>

#include "zen/backend.h"

/// To make the system easier to test, abstract the parts related to hardware
/// with this.
struct zn_backend_impl {
  struct zn_backend base;

  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;      // @nonnull, @owning
  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct wl_list input_device_list;  // zn_input_device_base::link

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
};

void zn_backend_impl_update_capabilities(struct zn_backend_impl *self);

struct zn_backend_impl *zn_backend_impl_get(struct zn_backend *base);

bool zn_backend_impl_start(struct zn_backend_impl *self);

struct zn_backend_impl *zn_backend_impl_create(struct wl_display *display);

void zn_backend_impl_destroy(struct zn_backend_impl *self);
