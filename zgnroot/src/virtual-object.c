#include "virtual-object.h"

#include <zen-common.h>
#include <zigen-protocol.h>

static void zgnr_virtual_object_destroy(struct zgnr_virtual_object_impl* self);

static void
zgnr_virtual_object_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_virtual_object_impl* self = resource->data;
  zgnr_virtual_object_destroy(self);
}

static void
zgnr_virtual_object_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_virtual_object_protocol_commit(
    struct wl_client* client, struct wl_resource* resource)
{
  // TODO:
  UNUSED(client);
  UNUSED(resource);
}

static void
zgnr_virtual_object_protocol_frame(
    struct wl_client* client, struct wl_resource* resource, uint32_t callback)
{
  // TODO:
  UNUSED(client);
  UNUSED(resource);
  UNUSED(callback);
}

static const struct zgn_virtual_object_interface interface = {
    .destroy = zgnr_virtual_object_protocol_destroy,
    .commit = zgnr_virtual_object_protocol_commit,
    .frame = zgnr_virtual_object_protocol_frame,
};

struct zgnr_virtual_object_impl*
zgnr_virtual_object_create(struct wl_client* client, uint32_t id)
{
  struct zgnr_virtual_object_impl* self;
  struct wl_resource* resource;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed allocate memory");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_virtual_object_interface, 1, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      resource, &interface, self, zgnr_virtual_object_handle_destroy);

  wl_signal_init(&self->base.events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_virtual_object_destroy(struct zgnr_virtual_object_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
