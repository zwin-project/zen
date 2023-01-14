#pragma once

#include <wayland-server-core.h>

struct zwnr_backend_impl;

struct zwnr_compositor {
  struct wl_global *global;  // null when inactive

  struct wl_display *display;
  struct zwnr_backend_impl *backend;  // nonnull, reference
};

/**
 * @return 0 if successful, -1 otherwise
 */
int zwnr_compositor_activate(struct zwnr_compositor *self);

void zwnr_compositor_deactivate(struct zwnr_compositor *self);

/**
 * When it's active, wl_registry shows a zwn_compositor global to clients.
 * Initially it's inactive.
 */
struct zwnr_compositor *zwnr_compositor_create(
    struct wl_display *display, struct zwnr_backend_impl *backend);

void zwnr_compositor_destroy(struct zwnr_compositor *self);
