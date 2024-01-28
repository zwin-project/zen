#pragma once

#include <wayland-server-core.h>

struct zn_virtual_object;
struct zn_xr_dispatcher;

struct zn_client_virtual_object {
  struct wl_resource *resource;  // @nonnull, @outlive

  struct zn_virtual_object *zn_virtual_object;  // @nonnull, @owning

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

struct zn_client_virtual_object *zn_client_virtual_object_create(
    struct wl_client *client, uint32_t id);

struct zn_client_virtual_object *zn_client_virtual_object_get(
    struct wl_resource *resource);
