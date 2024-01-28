#include "client-virtual-object.h"

#include <zwin-protocol.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/virtual-object.h"

static void zn_client_virtual_object_destroy(
    struct zn_client_virtual_object *self);

static void
zn_client_virtual_object_handle_destroy(struct wl_resource *resource)
{
  struct zn_client_virtual_object *self =
      zn_client_virtual_object_get(resource);

  if (self->zn_virtual_object->role_object != NULL) {
    wl_resource_post_error(self->resource,
        ZWN_VIRTUAL_OBJECT_ERROR_DEFUNCT_ROLE_OBJECT,
        "Virtual object is destroyed before its role object");
  }

  zn_client_virtual_object_destroy(self);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_virtual_object_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_virtual_object_protocol_commit(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  struct zn_client_virtual_object *self =
      zn_client_virtual_object_get(resource);

  if (self == NULL) {
    return;
  }

  zn_virtual_object_commit(self->zn_virtual_object);
}

/// @param resource can be inert (resource->user_data == NULL)
static void
zn_client_virtual_object_protocol_frame(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t callback UNUSED)
{}

static const struct zwn_virtual_object_interface implementation = {
    .destroy = zn_client_virtual_object_protocol_destroy,
    .commit = zn_client_virtual_object_protocol_commit,
    .frame = zn_client_virtual_object_protocol_frame,
};

struct zn_client_virtual_object *
zn_client_virtual_object_get(struct wl_resource *resource)
{
  return wl_resource_get_user_data(resource);
}

struct zn_client_virtual_object *
zn_client_virtual_object_create(struct wl_client *client, uint32_t id)
{
  struct zn_client_virtual_object *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->events.destroy);

  self->zn_virtual_object = zn_virtual_object_create();
  if (self->zn_virtual_object == NULL) {
    zn_error("Failed to get new zn_virtual_object");
    wl_client_post_no_memory(client);
    goto err_free;
  }

  self->resource =
      wl_resource_create(client, &zwn_virtual_object_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    goto err_zn_virtual_object;
  }

  wl_resource_set_implementation(self->resource, &implementation, self,
      &zn_client_virtual_object_handle_destroy);

  return self;

err_zn_virtual_object:
  zn_virtual_object_destroy(self->zn_virtual_object);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_client_virtual_object_destroy(struct zn_client_virtual_object *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  zn_virtual_object_destroy(self->zn_virtual_object);
  wl_resource_set_implementation(self->resource, &implementation, NULL, NULL);
  free(self);
}
