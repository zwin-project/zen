#pragma once

#include <wayland-server-core.h>

#include "zgnr/rendering-unit.h"

struct zgnr_virtual_object_impl;

/**
 * When the associated virtual object is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_rendering_unit_impl {
  struct zgnr_rendering_unit base;

  struct wl_resource* resource;
  struct wl_listener virtual_object_destroy_listener;
};

struct zgnr_rendering_unit_impl* zgnr_rendering_unit_create(
    struct wl_client* client, uint32_t id,
    struct zgnr_virtual_object_impl* virtual_object);