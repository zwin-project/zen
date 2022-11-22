#pragma once

#include "virtual-object.h"
#include "zgnr/bounded.h"

/**
 * When the associated virtual object is destroyed, this object is destroyed
 * and the wl_resource becomes inert (resource::data == NULL).
 */
struct zgnr_bounded_impl {
  struct zgnr_bounded base;
  struct zgnr_virtual_object_impl *virtual_object;  // nonnull

  struct wl_listener virtual_object_destroy_listener;

  struct wl_resource *resource;
};

struct zgnr_bounded_impl *zgnr_bounded_create(struct wl_client *client,
    uint32_t id, struct zgnr_virtual_object_impl *virtual_object);
