#include "space-manager.h"

#include <zen-common.h>
#include <zen-space-protocol.h>

#include "space.h"
#include "virtual-object.h"

static void
zgnr_space_manager_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_space_manager_protocol_get_space(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *virtual_object_resource)
{
  struct zgnr_space_manager_impl *self = wl_resource_get_user_data(resource);
  struct zgnr_space_impl *space;

  struct zgnr_virtual_object_impl *virtual_object =
      wl_resource_get_user_data(virtual_object_resource);

  space = zgnr_space_create(client, id, virtual_object);

  wl_signal_emit(&self->base.events.new_space, &space->base);
}

static const struct zen_space_manager_interface implementation = {
    .destroy = zgnr_space_manager_protocol_destroy,
    .get_space = zgnr_space_manager_protocol_get_space,
};

static void
zgnr_space_manager_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zgnr_space_manager_impl *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zen_space_manager_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zgnr_space_manager *
zgnr_space_manager_create(struct wl_display *display)
{
  struct zgnr_space_manager_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory ");
    goto err;
  }

  self->global = wl_global_create(
      display, &zen_space_manager_interface, 1, self, zgnr_space_manager_bind);
  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    goto err_free;
  }

  wl_signal_init(&self->base.events.new_space);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zgnr_space_manager_destroy(struct zgnr_space_manager *parent)
{
  struct zgnr_space_manager_impl *self = zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.new_space.listener_list);
  wl_global_remove(self->global);
  free(self);
}
