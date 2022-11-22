#include "bounded.h"

#include <zen-common.h>
#include <zigen-shell-protocol.h>

static void zgnr_bounded_destroy(struct zgnr_bounded_impl *self);
static void zgnr_bounded_inert(struct zgnr_bounded_impl *self);

static void
zgnr_bounded_handle_destroy(struct wl_resource *resource)
{
  struct zgnr_bounded_impl *self = wl_resource_get_user_data(resource);

  zgnr_bounded_destroy(self);
}

static void
zgnr_bounded_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_bounded_interface implementation = {
    .destroy = zgnr_bounded_protocol_destroy,
};

static void
zgnr_bounded_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_bounded_impl *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zgnr_bounded_inert(self);
}

static void
zgnr_bounded_inert(struct zgnr_bounded_impl *self)
{
  struct wl_resource *resource = self->resource;
  zgnr_bounded_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_bounded_impl *
zgnr_bounded_create(struct wl_client *client, uint32_t id,
    struct zgnr_virtual_object_impl *virtual_object)
{
  struct zgnr_bounded_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->virtual_object = virtual_object;

  wl_signal_init(&self->base.events.destroy);

  self->virtual_object_destroy_listener.notify =
      zgnr_bounded_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  self->resource = wl_resource_create(client, &zgn_bounded_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zgnr_bounded_handle_destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_bounded_destroy(struct zgnr_bounded_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->virtual_object_destroy_listener.link);
  wl_list_remove(&self->base.events.destroy.listener_list);

  UNUSED(self);
}
