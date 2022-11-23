#include "shell.h"

#include <cglm/vec3.h>
#include <zen-common.h>
#include <zigen-shell-protocol.h>

#include "bounded.h"
#include "virtual-object.h"

static void
zgnr_shell_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zgnr_shell_protocol_get_bounded(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object_resource,
    struct wl_array* half_size_array)
{
  struct zgnr_shell_impl* self = wl_resource_get_user_data(resource);
  vec3 half_size;

  struct zgnr_virtual_object_impl* virtual_object =
      wl_resource_get_user_data(virtual_object_resource);

  struct zgnr_bounded_impl* bounded =
      zgnr_bounded_create(client, id, virtual_object);

  if (!zgnr_virtual_object_set_role(virtual_object,
          ZGNR_VIRTUAL_OBJECT_ROLE_BOUNDED, &bounded->base, bounded->resource,
          ZGN_SHELL_ERROR_ROLE)) {
    return;
  }

  zn_array_to_vec3(half_size_array, half_size);

  zgnr_bounded_configure(bounded, half_size);

  wl_signal_emit(&self->base.events.new_bounded, &bounded->base);
}

static const struct zgn_shell_interface implementation = {
    .destroy = zgnr_shell_protocol_destroy,
    .get_bounded = zgnr_shell_protocol_get_bounded,
};

static void
zgnr_shell_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zgnr_shell_impl* self = data;

  struct wl_resource* resource =
      wl_resource_create(client, &zgn_shell_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zgnr_shell*
zgnr_shell_create(struct wl_display* display)
{
  struct zgnr_shell_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global =
      wl_global_create(display, &zgn_shell_interface, 1, self, zgnr_shell_bind);
  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    goto err_free;
  }

  wl_signal_init(&self->base.events.new_bounded);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zgnr_shell_destroy(struct zgnr_shell* parent)
{
  struct zgnr_shell_impl* self = zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.new_bounded.listener_list);

  wl_global_remove(self->global);

  free(self);
}
