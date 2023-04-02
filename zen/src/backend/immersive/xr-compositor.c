#include "xr-compositor.h"

#include <zwin-protocol.h>

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"
#include "zen/xr-system.h"

static void
zn_xr_compositor_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zn_xr_compositor_handle_xr_system_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xr_compositor *self =
      zn_container_of(listener, self, xr_system_destroy_listener);

  zn_xr_compositor_destroy(self);
}

static void
zn_xr_compositor_protocol_create_virtual_object(struct wl_client *client UNUSED,
    struct wl_resource *resource UNUSED, uint32_t id UNUSED)
{}

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

  wl_resource_set_implementation(
      resource, &implementation, self, zn_xr_compositor_handle_destroy);

  wl_list_insert(&self->resource_list, wl_resource_get_link(resource));
}

struct zn_xr_compositor *
zn_xr_compositor_create(
    struct wl_display *display UNUSED, struct zn_xr_system *xr_system)
{
  struct zn_xr_compositor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xr_system = xr_system;
  wl_signal_init(&self->events.destroy);
  wl_list_init(&self->resource_list);

  self->global = wl_global_create(
      display, &zwn_compositor_interface, 1, self, zn_xr_compositor_bind);
  if (self->global == NULL) {
    zn_error("Failed to create zwn_compositor global");
    goto err_free;
  }

  self->xr_system_destroy_listener.notify =
      zn_xr_compositor_handle_xr_system_destroy;
  wl_signal_add(&xr_system->events.destroy, &self->xr_system_destroy_listener);

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
  wl_list_remove(&self->resource_list);
  wl_list_remove(&self->xr_system_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}