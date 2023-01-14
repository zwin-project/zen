#pragma once

#include <wayland-server-core.h>

#include "zwnr/virtual-object.h"

struct zwnr_virtual_object_impl {
  struct zwnr_virtual_object base;
  struct wl_display *display;

  struct {
    struct wl_list frame_callback_list;
  } pending;

  struct {
    /** Emitted before the public signal (base::events::committed) */
    struct wl_signal on_commit;  // (NULL)
  } events;
};

/**
 * @return true if successful
 * @param role_object must be null when role == ZWNR_VIRTUAL_OBJECT_ROLE_NONE
 * @param error_resource can be null when role == ZWNR_VIRTUAL_OBJECT_ROLE_NONE
 */
bool zwnr_virtual_object_set_role(struct zwnr_virtual_object_impl *self,
    enum zwnr_virtual_object_role role, void *role_object,
    struct wl_resource *error_resource, uint32_t error_code);

struct zwnr_virtual_object_impl *zwnr_virtual_object_create(
    struct wl_client *client, uint32_t id, struct wl_display *display);
