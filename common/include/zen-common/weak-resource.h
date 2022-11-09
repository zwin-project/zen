#pragma once

#include <wayland-server-core.h>

/**
 * This has a weak link to a wl_resource and handles deletion of the wl_resource
 * appropriately.
 */
struct zn_weak_resource {
  struct wl_resource *resource;
  struct wl_listener destroy_listener;
};

/** When releasing zn_weak_resource, call unlink method */
void zn_weak_resource_init(struct zn_weak_resource *self);

void *zn_weak_resource_get_user_data(struct zn_weak_resource *self);

void zn_weak_resource_link(
    struct zn_weak_resource *self, struct wl_resource *resource);

void zn_weak_resource_unlink(struct zn_weak_resource *self);
