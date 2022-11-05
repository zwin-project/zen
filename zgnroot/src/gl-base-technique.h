#pragma once

#include <wayland-server-core.h>

#include "rendering-unit.h"
#include "zgnr/gl-base-technique.h"

/**
 * When the associated rendering unit is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_gl_base_technique_impl {
  struct zgnr_gl_base_technique base;

  struct wl_resource *resource;
  struct wl_listener rendering_unit_destroy_listener;
};

struct zgnr_gl_base_technique_impl *zgnr_gl_base_technique_create(
    struct wl_client *client, uint32_t id,
    struct zgnr_rendering_unit_impl *unit);
