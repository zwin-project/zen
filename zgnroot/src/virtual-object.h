#pragma once

#include <wayland-server-core.h>

#include "zgnr/virtual-object.h"

struct zgnr_virtual_object_impl {
  struct zgnr_virtual_object base;

  struct {
    struct wl_list frame_callback_list;
  } pending;

  struct {
    /** Emitted before the public signal (base::events::committed) */
    struct wl_signal on_commit;  // (NULL)
  } events;
};

struct zgnr_virtual_object_impl* zgnr_virtual_object_create(
    struct wl_client* client, uint32_t id);
