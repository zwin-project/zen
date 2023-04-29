#pragma once

#include <wayland-server-core.h>

struct zn_gl_vertex_array;

struct zn_client_gl_vertex_array {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_vertex_array *zn_gl_vertex_array;  // @nonnull, @outlive

  struct wl_listener zn_gl_vertex_array_destroy_listener;
};

struct zn_client_gl_vertex_array *zn_client_gl_vertex_array_create(
    struct wl_client *client, uint32_t id);

/// @return value is nullable
struct zn_client_gl_vertex_array *zn_client_gl_vertex_array_get(
    struct wl_resource *resource);
