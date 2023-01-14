#include "shell.h"

#include <cglm/types.h>
#include <zen-common.h>
#include <zwin-protocol.h>
#include <zwin-shell-protocol.h>

#include "bounded.h"
#include "expansive.h"
#include "virtual-object.h"

static void
zwnr_shell_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zwnr_shell_protocol_get_bounded(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *virtual_object_resource,
    struct wl_array *half_size_array)
{
  struct zwnr_shell_impl *self = wl_resource_get_user_data(resource);
  vec3 half_size;

  if (zn_array_to_vec3(half_size_array, half_size) != 0) {
    wl_resource_post_error(resource, ZWN_COMPOSITOR_ERROR_WL_ARRAY_SIZE,
        "half_size is expected vec3 (%ld bytes) but got %ld bytes",
        sizeof(vec3), half_size_array->size);
    return;
  }

  struct zwnr_virtual_object_impl *virtual_object =
      wl_resource_get_user_data(virtual_object_resource);

  struct zwnr_bounded_impl *bounded =
      zwnr_bounded_create(client, id, virtual_object);

  if (!zwnr_virtual_object_set_role(virtual_object,
          ZWNR_VIRTUAL_OBJECT_ROLE_BOUNDED, &bounded->base, bounded->resource,
          ZWN_SHELL_ERROR_ROLE)) {
    return;
  }

  zwnr_bounded_configure(bounded, half_size);

  wl_signal_emit(&self->base.events.new_bounded, &bounded->base);
}

static void
zwnr_shell_protocol_get_expansive(struct wl_client *client,
    struct wl_resource *resource, uint32_t id,
    struct wl_resource *virtual_object_resource)
{
  struct zwnr_shell_impl *self = wl_resource_get_user_data(resource);

  struct zwnr_virtual_object_impl *virtual_object =
      wl_resource_get_user_data(virtual_object_resource);

  struct zwnr_expansive_impl *expansive =
      zwnr_expansive_create(client, id, virtual_object);

  if (!zwnr_virtual_object_set_role(virtual_object,
          ZWNR_VIRTUAL_OBJECT_ROLE_EXPANSIVE, &expansive->base,
          expansive->resource, ZWN_SHELL_ERROR_ROLE)) {
    return;
  }

  wl_signal_emit(&self->base.events.new_expansive, &expansive->base);
}

static const struct zwn_shell_interface implementation = {
    .destroy = zwnr_shell_protocol_destroy,
    .get_bounded = zwnr_shell_protocol_get_bounded,
    .get_expansive = zwnr_shell_protocol_get_expansive,
};

static void
zwnr_shell_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zwnr_shell_impl *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_shell_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zwnr_shell *
zwnr_shell_create(struct wl_display *display)
{
  struct zwnr_shell_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global =
      wl_global_create(display, &zwn_shell_interface, 1, self, zwnr_shell_bind);
  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    goto err_free;
  }

  wl_signal_init(&self->base.events.new_bounded);
  wl_signal_init(&self->base.events.new_expansive);

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zwnr_shell_destroy(struct zwnr_shell *parent)
{
  struct zwnr_shell_impl *self = zn_container_of(parent, self, base);

  wl_list_remove(&self->base.events.new_bounded.listener_list);
  wl_list_remove(&self->base.events.new_expansive.listener_list);

  wl_global_remove(self->global);

  free(self);
}
