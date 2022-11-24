#include "compositor.h"

#include <wayland-server-core.h>
#include <zen-common.h>
#include <zigen-protocol.h>

#include "backend.h"
#include "region.h"
#include "virtual-object.h"

static void
zgnr_compositor_protocol_create_virtual_object(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_compositor* self = wl_resource_get_user_data(resource);
  struct zgnr_virtual_object_impl* virtual_object =
      zgnr_virtual_object_create(client, id, self->display);

  if (virtual_object == NULL) {
    zn_error("Failed to create zgnr_virtual_object");
    wl_client_post_no_memory(client);
    return;
  }

  wl_signal_emit(
      &self->backend->base.events.new_virtual_object, &virtual_object->base);
}

static void
zgnr_compositor_protocol_create_region(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(resource);
  struct zgnr_region* region = zgnr_region_create(client, id);
  if (region == NULL) {
    zn_error("Failed to create zgnr_region");
    wl_client_post_no_memory(client);
  }
}

static const struct zgn_compositor_interface implementation = {
    .create_virtual_object = zgnr_compositor_protocol_create_virtual_object,
    .create_region = zgnr_compositor_protocol_create_region,
};

static void
zgnr_compositor_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zgnr_compositor* self = data;

  struct wl_resource* resource =
      wl_resource_create(client, &zgn_compositor_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to create a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

int
zgnr_compositor_activate(struct zgnr_compositor* self)
{
  if (self->global != NULL) return 0;

  self->global = wl_global_create(
      self->display, &zgn_compositor_interface, 1, self, zgnr_compositor_bind);

  if (self->global == NULL) {
    zn_error("Failed to create wl_global");
    return -1;
  }

  return 0;
}

void
zgnr_compositor_deactivate(struct zgnr_compositor* self)
{
  if (self->global == NULL) return;

  wl_global_destroy(self->global);
  self->global = NULL;

  return;
}

struct zgnr_compositor*
zgnr_compositor_create(
    struct wl_display* display, struct zgnr_backend_impl* backend)
{
  struct zgnr_compositor* self;

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
zgnr_compositor_destroy(struct zgnr_compositor* self)
{
  if (self->global) {
    wl_global_destroy(self->global);
  }

  free(self);
}
