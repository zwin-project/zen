#pragma once

#include <wayland-server-core.h>

struct zn_shm_buffer;
struct zn_gl_shader;

struct zn_client_gl_shader {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_shader *zn_gl_shader;  // @nonnull, @outlive

  struct wl_listener zn_gl_shader_destroy_listener;
};

struct zn_client_gl_shader *zn_client_gl_shader_create(struct wl_client *client,
    uint32_t id, struct zn_shm_buffer *buffer, uint32_t type);
