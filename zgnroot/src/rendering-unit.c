#include "rendering-unit.h"

#include <zen-common.h>
#include <zigen-gles-v32-protocol.h>

#include "virtual-object.h"

static void zgnr_rendering_unit_destroy(struct zgnr_rendering_unit_impl* self);
static void zgnr_rendering_unit_inert(struct zgnr_rendering_unit_impl* self);

static void
zgnr_rendering_unit_handle_destroy(struct wl_resource* resource)
{
  struct zgnr_rendering_unit_impl* self = wl_resource_get_user_data(resource);
  zgnr_rendering_unit_destroy(self);
}

static void
zgnr_rendering_unit_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

/** Be careful, resource can be inert. */
static const struct zgn_rendering_unit_interface interface = {
    .destroy = zgnr_rendering_unit_protocol_destroy,
};

static void
zgnr_rendering_unit_handle_virtual_object_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zgnr_rendering_unit_impl* self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zgnr_rendering_unit_inert(self);
}

static void
zgnr_rendering_unit_inert(struct zgnr_rendering_unit_impl* self)
{
  struct wl_resource* resource = self->resource;
  zgnr_rendering_unit_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_rendering_unit_impl*
zgnr_rendering_unit_create(struct wl_client* client, uint32_t id,
    struct zgnr_virtual_object_impl* virtual_object)
{
  struct zgnr_rendering_unit_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->resource =
      wl_resource_create(client, &zgn_rendering_unit_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &interface, self, zgnr_rendering_unit_handle_destroy);

  self->base.virtual_object = &virtual_object->base;

  wl_signal_init(&self->base.events.destroy);

  self->virtual_object_destroy_listener.notify =
      zgnr_rendering_unit_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_rendering_unit_destroy(struct zgnr_rendering_unit_impl* self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);
  free(self);
}
