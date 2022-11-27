#include "seat-ray.h"

#include <zen-common.h>
#include <zigen-protocol.h>

#include "seat.h"

static void zgnr_seat_ray_destroy(struct zgnr_seat_ray *self);

static void
zgnr_seat_ray_handle_destroy(struct wl_resource *resource)
{
  struct zgnr_seat_ray *self = wl_resource_get_user_data(resource);

  zgnr_seat_ray_destroy(self);
}

static void
zgnr_seat_ray_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_ray_interface implementation = {
    .release = zgnr_seat_ray_protocol_release,
};

struct zgnr_seat_ray *
zgnr_seat_ray_create(
    struct wl_client *client, uint32_t id, struct zgnr_seat_impl *seat)
{
  struct zgnr_seat_ray *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->resource = wl_resource_create(client, &zgn_ray_interface, 1, id);
  if (self->resource == NULL) {
    goto err_free;
  }

  wl_resource_set_implementation(
      self->resource, &implementation, self, &zgnr_seat_ray_handle_destroy);

  self->seat = seat;
  wl_list_insert(&seat->seat_ray_list, &self->link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zgnr_seat_ray_destroy(struct zgnr_seat_ray *self)
{
  wl_list_remove(&self->link);
  free(self);
}
