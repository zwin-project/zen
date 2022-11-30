#pragma once

#include <wayland-server-core.h>

struct zgnr_backend_impl;

struct zgnr_compositor {
  struct wl_global *global;  // null when inactive

  struct wl_display *display;
  struct zgnr_backend_impl *backend;  // nonnull, reference
};

/**
 * @return 0 if successful, -1 otherwise
 */
int zgnr_compositor_activate(struct zgnr_compositor *self);

void zgnr_compositor_deactivate(struct zgnr_compositor *self);

/**
 * When it's active, wl_registry shows a zgn_compositor global to clients.
 * Initially it's inactive.
 */
struct zgnr_compositor *zgnr_compositor_create(
    struct wl_display *display, struct zgnr_backend_impl *backend);

void zgnr_compositor_destroy(struct zgnr_compositor *self);
