#pragma once

#include <wayland-server-core.h>

#include "rendering-unit.h"
#include "weak-resource.h"
#include "zgnr/gl-base-technique.h"

/**
 * When the associated rendering unit is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_gl_base_technique_impl {
  struct zgnr_gl_base_technique base;

  struct wl_resource *resource;

  struct {
    struct zgnr_weak_resource vertex_array;  // zgnr_gl_vertex_array_impl
  } pending;

  struct wl_listener rendering_unit_destroy_listener;
  struct wl_listener rendering_unit_commit_listener;

  struct wl_listener current_vertex_array_destroy_listener;
};

struct zgnr_gl_base_technique_impl *zgnr_gl_base_technique_create(
    struct wl_client *client, uint32_t id,
    struct zgnr_rendering_unit_impl *unit);
