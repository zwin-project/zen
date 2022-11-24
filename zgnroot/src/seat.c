#include "seat.h"

#include <zen-common.h>
#include <zigen-protocol.h>

static void
zgnr_seat_protocol_get_ray(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zgnr_seat_protocol_release(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_seat_interface implementation = {
    .get_ray = zgnr_seat_protocol_get_ray,
    .release = zgnr_seat_protocol_release,
};

static void
zgnr_seat_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zgnr_seat_impl* self = data;

  struct wl_resource* resource =
      wl_resource_create(client, &zgn_seat_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to creat a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(resource, &implementation, self, NULL);
}

struct zgnr_seat*
zgnr_seat_create(struct wl_display* display)
{
  struct zgnr_seat_impl* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global =
      wl_global_create(display, &zgn_seat_interface, 1, self, zgnr_seat_bind);

  return &self->base;

err:
  return NULL;
}

void
zgnr_seat_destroy(struct zgnr_seat* parent)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);

  wl_global_destroy(self->global);

  free(self);
}
