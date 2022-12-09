#include "space.h"

#include <zen-common.h>
#include <zen-space-protocol.h>

static void zgnr_space_destroy(struct zgnr_space_impl *self);
static void zgnr_space_inert(struct zgnr_space_impl *self);

void
zgnr_space_enter(struct zgnr_space *parent)
{
  struct zgnr_space_impl *self = zn_container_of(parent, self, base);

  zen_space_send_enter(self->resource);
}

void
zgnr_space_leave(struct zgnr_space *parent)
{
  struct zgnr_space_impl *self = zn_container_of(parent, self, base);

  zen_space_send_leave(self->resource);
}

void
zgnr_space_shutdown(struct zgnr_space *parent)
{
  struct zgnr_space_impl *self = zn_container_of(parent, self, base);

  zen_space_send_shutdown(self->resource);
}

static void
zgnr_space_handle_destroy(struct wl_resource *resource)
{
  struct zgnr_space_impl *self = wl_resource_get_user_data(resource);

  zgnr_space_destroy(self);
}

static void
zgnr_space_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zen_space_interface implementation = {
    .destroy = zgnr_space_protocol_destroy,
};

static void
zgnr_space_handle_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zgnr_space_impl *self =
      zn_container_of(listener, self, virtual_object_destroy_listener);

  zgnr_space_inert(self);
}

static void
zgnr_space_inert(struct zgnr_space_impl *self)
{
  struct wl_resource *resource = self->resource;
  zgnr_space_destroy(self);
  wl_resource_set_user_data(resource, NULL);
  wl_resource_set_destructor(resource, NULL);
}

struct zgnr_space_impl *
zgnr_space_create(struct wl_client *client, uint32_t id,
    struct zgnr_virtual_object_impl *virtual_object)
{
  struct zgnr_space_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate a memory");
    goto err;
  }

  self->base.virtual_object = &virtual_object->base;

  wl_signal_init(&self->base.events.destroy);

  self->resource = wl_resource_create(client, &zen_space_interface, 1, id);
  if (self->resource == NULL) {
    zn_error("Failed to create a wl_resource");
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, zgnr_space_handle_destroy);

  self->virtual_object_destroy_listener.notify =
      zgnr_space_handle_virtual_object_destroy;
  wl_signal_add(&virtual_object->base.events.destroy,
      &self->virtual_object_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_space_destroy(struct zgnr_space_impl *self)
{
  wl_signal_emit(&self->base.events.destroy, NULL);

  wl_list_remove(&self->base.events.destroy.listener_list);
  wl_list_remove(&self->virtual_object_destroy_listener.link);
  free(self);
}
