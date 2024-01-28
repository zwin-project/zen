#include "xr-compositor.h"

#include <zwin-protocol.h>

#include "client-virtual-object.h"
#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/xr-system.h"

static void
zn_xr_compositor_protocol_create_virtual_object(
    struct wl_client *client, struct wl_resource *resource UNUSED, uint32_t id)
{
  zn_client_virtual_object_create(client, id);
}

static void
zn_xr_compositor_protocol_create_region(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t id UNUSED)
{}

static const struct zwn_compositor_interface implementation = {
    .create_virtual_object = zn_xr_compositor_protocol_create_virtual_object,
    .create_region = zn_xr_compositor_protocol_create_region,
};

static void
zn_xr_compositor_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zn_xr_compositor *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_compositor_interface, (int)version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zn_xr_compositor *
zn_xr_compositor_create(struct wl_display *display UNUSED)
{
  struct zn_xr_compositor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_signal_init(&self->events.destroy);

  self->global = wl_global_create(
      display, &zwn_compositor_interface, 1, self, zn_xr_compositor_bind);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_compositor global");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_xr_compositor_destroy(struct zn_xr_compositor *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_global_destroy(self->global);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
