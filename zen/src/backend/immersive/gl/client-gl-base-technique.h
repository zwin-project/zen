#pragma once

#include <wayland-server-core.h>

struct zn_client_gl_rendering_unit;
struct zn_gl_base_technique;

struct zn_client_gl_base_technique {
  struct zn_client_gl_rendering_unit *rendering_unit;  // @nonnull @outlive

  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_gl_base_technique *zn_gl_base_technique;  // @nonnull, @outlive

  struct wl_listener rendering_unit_destroy_listener;
  struct wl_listener zn_gl_base_technique_destroy_listener;
};

struct zn_client_gl_base_technique *zn_client_gl_base_technique_create(
    struct wl_client *client, uint32_t id,
    struct zn_client_gl_rendering_unit *rendering_unit);

/// @return value is nullable
struct zn_client_gl_base_technique *zn_client_gl_base_technique_get(
    struct wl_resource *resource);
