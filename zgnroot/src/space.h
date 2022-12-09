#pragma once

#include "virtual-object.h"
#include "zgnr/space.h"

/**
 * When the associated virtual object is destroyed, this object is destroyed
 * and the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_space_impl {
  struct zgnr_space base;

  struct wl_resource *resource;

  struct wl_listener virtual_object_destroy_listener;
};

struct zgnr_space_impl *zgnr_space_create(struct wl_client *client, uint32_t id,
    struct zgnr_virtual_object_impl *virtual_object);
