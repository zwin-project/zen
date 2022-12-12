#pragma once

#include "virtual-object.h"
#include "zgnr/expansive.h"

/**
 * When the associated virtual object is destroyed, this object is destroyed
 * and the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_expansive_impl {
  struct zgnr_expansive base;

  struct wl_resource *resource;

  struct wl_listener virtual_object_destroy_listener;
};

struct zgnr_expansive_impl *zgnr_expansive_create(struct wl_client *client,
    uint32_t id, struct zgnr_virtual_object_impl *virtual_object);
