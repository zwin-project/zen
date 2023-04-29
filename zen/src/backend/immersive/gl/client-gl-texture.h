#pragma once

#include <wayland-server-core.h>

struct zn_gl_texture;

struct zn_client_gl_texture {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_texture *zn_gl_texture;  // @nonnull, @outlive

  struct wl_listener zn_gl_texture_destroy_listener;
};

struct zn_client_gl_texture *zn_client_gl_texture_create(
    struct wl_client *client, uint32_t id);

/// @return value is nullable
struct zn_client_gl_texture *zn_client_gl_texture_get(
    struct wl_resource *resource);
