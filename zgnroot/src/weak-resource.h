#pragma once

#include <wayland-server-core.h>

/**
 * This has a weak link to a wl_resource and handles deletion of the wl_resource
 * appropriately.
 */
struct zgnr_weak_resource {
  struct wl_resource *resource;
  struct wl_listener destroy_listener;
};

/** When releasing zgnr_weak_resource, call unlink method */
void zgnr_weak_resource_init(struct zgnr_weak_resource *self);

void *zgnr_weak_resource_get_user_data(struct zgnr_weak_resource *self);

void zgnr_weak_resource_link(
    struct zgnr_weak_resource *self, struct wl_resource *resource);

void zgnr_weak_resource_unlink(struct zgnr_weak_resource *self);
