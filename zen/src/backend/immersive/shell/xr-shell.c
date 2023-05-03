#include "backend/immersive/xr-shell.h"

#include <zwin-protocol.h>
#include <zwin-shell-protocol.h>

#include "backend/immersive/client-virtual-object.h"
#include "bounded.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/virtual-object.h"

static void
zn_xr_shell_protocol_destroy(
    struct wl_client *client UNUSED, struct wl_resource *resource)
{
  wl_resource_destroy(resource);
}

static void
zn_xr_shell_protocol_get_bounded(struct wl_client *client,
    struct wl_resource *resource UNUSED, uint32_t id,
    struct wl_resource *virtual_object_resource)
{
  struct zn_client_virtual_object *client_virtual_object =
      zn_client_virtual_object_get(virtual_object_resource);

  if (client_virtual_object == NULL) {
    return;
  }

  struct zn_bounded *bounded =
      zn_bounded_create(client, id, client_virtual_object->zn_virtual_object);
  if (bounded == NULL) {
    zn_error("Failed to create a zn_bounded");
    return;
  }

  if (!zn_virtual_object_set_role(client_virtual_object->zn_virtual_object,
          ZN_VIRTUAL_OBJECT_ROLE_BOUNDED, bounded)) {
    wl_resource_post_error(client_virtual_object->resource,
        ZWN_VIRTUAL_OBJECT_ERROR_ROLE,
        "Failed assign bounded role to the virtual object (%d)",
        wl_resource_get_id(client_virtual_object->resource));
    wl_resource_destroy(bounded->resource);
  }
}

static void
zn_xr_shell_protocol_get_expansive(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t id UNUSED,
    struct wl_resource *virtual_object_resource UNUSED)
{}

static const struct zwn_shell_interface implementation = {
    .destroy = zn_xr_shell_protocol_destroy,
    .get_bounded = zn_xr_shell_protocol_get_bounded,
    .get_expansive = zn_xr_shell_protocol_get_expansive,
};

static void
zn_xr_shell_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_xr_shell *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_shell_interface, (int)version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zn_xr_shell *
zn_xr_shell_create(struct wl_display *display)
{
  struct zn_xr_shell *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global = wl_global_create(
      display, &zwn_shell_interface, 1, self, zn_xr_shell_bind);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_shell global");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_xr_shell_destroy(struct zn_xr_shell *self)
{
  wl_global_destroy(self->global);
  free(self);
}
