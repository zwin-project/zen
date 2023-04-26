#pragma once

#include <wayland-server-core.h>

struct zn_gl_program;

struct zn_client_gl_program {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_program *zn_gl_program;  // @nonnull, @outlive

  struct wl_listener zn_gl_program_destroy_listener;
};

struct zn_client_gl_program *zn_client_gl_program_create(
    struct wl_client *client, uint32_t id);

/// @return value is nullable
struct zn_client_gl_program *zn_client_gl_program_get(
    struct wl_resource *resource);
