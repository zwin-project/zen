#pragma once

#include <wayland-server-core.h>

#include "virtual-object.h"
#include "zgnr/rendering-unit.h"

struct zgnr_gl_base_technique_impl;

/**
 * When the associated virtual object is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_rendering_unit_impl {
  struct zgnr_rendering_unit base;

  struct wl_resource* resource;

  struct {
    struct wl_signal on_commit;
  } events;

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener virtual_object_commit_listener;
  struct wl_listener current_technique_destroy_listener;
};

void zgnr_rendering_unit_set_current_technique(
    struct zgnr_rendering_unit_impl* self,
    struct zgnr_gl_base_technique_impl* technique);

struct zgnr_rendering_unit_impl* zgnr_rendering_unit_create(
    struct wl_client* client, uint32_t id,
    struct zgnr_virtual_object_impl* virtual_object);
