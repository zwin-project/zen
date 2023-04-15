#pragma once

#include <wayland-server-core.h>

struct zn_gl_buffer;

struct zn_client_gl_buffer {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_buffer *zn_gl_buffer;  // @nonnull, @outlive

  struct wl_listener zn_gl_buffer_destroy_listener;
};

struct zn_client_gl_buffer *zn_client_gl_buffer_create(
    struct wl_client *client, uint32_t id);
