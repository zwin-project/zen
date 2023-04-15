#pragma once

#include <wayland-server-core.h>

struct zn_client_virtual_object;
struct zn_gl_rendering_unit;

struct zn_client_gl_rendering_unit {
  struct zn_client_virtual_object *virtual_object;  // @nonnull, @outlive

  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_rendering_unit *zn_gl_rendering_unit;  // @nonnull, @outlive

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener zn_gl_rendering_unit_destroy_listener;
};

struct zn_client_gl_rendering_unit *zn_client_gl_rendering_unit_create(
    struct wl_client *client, uint32_t id,
    struct zn_client_virtual_object *virtual_object);
