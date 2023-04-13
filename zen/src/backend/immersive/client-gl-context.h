#pragma once

#include <wayland-server.h>

struct zn_xr_compositor;

struct zn_client_gl_context {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_xr_compositor *xr_compositor;  // @nonnull, @outlive

  struct wl_listener xr_compositor_destroy_listener;
};

struct zn_client_gl_context *zn_client_gl_context_create(
    struct wl_client *client, uint32_t id,
    struct zn_xr_compositor *xr_compositor);

void zn_client_gl_context_destroy(struct zn_client_gl_context *self);
