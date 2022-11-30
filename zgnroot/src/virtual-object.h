#pragma once

#include <wayland-server-core.h>

#include "zgnr/virtual-object.h"

struct zgnr_virtual_object_impl {
  struct zgnr_virtual_object base;
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
 * @param role_object must be null when role == ZGNR_VIRTUAL_OBJECT_ROLE_NONE
 * @param error_resource can be null when role == ZGNR_VIRTUAL_OBJECT_ROLE_NONE
 */
bool zgnr_virtual_object_set_role(struct zgnr_virtual_object_impl *self,
    enum zgnr_virtual_object_role role, void *role_object,
    struct wl_resource *error_resource, uint32_t error_code);

struct zgnr_virtual_object_impl *zgnr_virtual_object_create(
    struct wl_client *client, uint32_t id, struct wl_display *display);
