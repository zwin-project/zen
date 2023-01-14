#include "compositor.h"

#include <wayland-server-core.h>
#include <zen-common.h>
#include <zwin-protocol.h>

#include "backend.h"
#include "region.h"
#include "virtual-object.h"

static void
zwnr_compositor_protocol_create_virtual_object(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  struct zwnr_compositor *self = wl_resource_get_user_data(resource);
  struct zwnr_virtual_object_impl *virtual_object =
      zwnr_virtual_object_create(client, id, self->display);

  if (virtual_object == NULL) {
    zn_error("Failed to create zwnr_virtual_object");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(
      &self->backend->base.events.new_virtual_object, &virtual_object->base);
}

static void
zwnr_compositor_protocol_create_region(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  UNUSED(resource);
  struct zwnr_region *region = zwnr_region_create(client, id);
  if (region == NULL) {
    zn_error("Failed to create zwnr_region");
    wl_client_post_no_memory(client);
  }
}

static const struct zwn_compositor_interface implementation = {
    .create_virtual_object = zwnr_compositor_protocol_create_virtual_object,
    .create_region = zwnr_compositor_protocol_create_region,
};

static void
zwnr_compositor_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zwnr_compositor *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_compositor_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

int
zwnr_compositor_activate(struct zwnr_compositor *self)
{
  if (self->global != NULL) return 0;

  self->global = wl_global_create(
      self->display, &zwn_compositor_interface, 1, self, zwnr_compositor_bind);

  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    return -1;
  }

  return 0;
}

void
zwnr_compositor_deactivate(struct zwnr_compositor *self)
{
  if (self->global == NULL) return;

  wl_global_destroy(self->global);
  self->global = NULL;

  return;
}

struct zwnr_compositor *
zwnr_compositor_create(
    struct wl_display *display, struct zwnr_backend_impl *backend)
{
  struct zwnr_compositor *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global = NULL;
  self->display = display;
  self->backend = backend;

  return self;

err:
  return NULL;
}

void
zwnr_compositor_destroy(struct zwnr_compositor *self)
{
  if (self->global) {
    wl_global_destroy(self->global);
  }

  free(self);
}
