#pragma once

#include <wayland-server-core.h>

#include "virtual-object.h"
#include "zwnr/rendering-unit.h"

struct zwnr_gl_base_technique_impl;

/**
 * When the associated virtual object is destroyed, this object is destroyed and
 * the wl_resource becomes inert (resource::data == NULL).
 */
struct zwnr_rendering_unit_impl {
  struct zwnr_rendering_unit base;

  struct wl_resource *resource;

  struct {
    struct wl_signal on_commit;
  } events;

  struct wl_listener virtual_object_destroy_listener;
  struct wl_listener virtual_object_commit_listener;
  struct wl_listener current_technique_destroy_listener;
};

void zwnr_rendering_unit_set_current_technique(
    struct zwnr_rendering_unit_impl *self,
    struct zwnr_gl_base_technique_impl *technique);

struct zwnr_rendering_unit_impl *zwnr_rendering_unit_create(
    struct wl_client *client, uint32_t id,
    struct zwnr_virtual_object_impl *virtual_object);
