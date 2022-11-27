#include "seat.h"

#include <zen-common.h>
#include <zigen-protocol.h>

#include "seat-ray.h"
#include "virtual-object.h"

void
zgnr_seat_set_capabilities(struct zgnr_seat* parent, uint32_t capabilities)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);
  struct wl_resource* resource;

  self->base.capabilities = capabilities;
  wl_resource_for_each (resource, &self->resource_list) {
    zgn_seat_send_capabilities(resource, capabilities);
  }
}

void
zgnr_seat_send_ray_enter(struct zgnr_seat* parent,
    struct zgnr_virtual_object* virtual_object, vec3 origin, vec3 direction)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);
  struct wl_client* client = wl_resource_get_client(virtual_object->resource);
  struct wl_display* display = wl_global_get_display(self->global);

  uint32_t serial = wl_display_next_serial(display);

  struct wl_array origin_array;
  struct wl_array direction_array;
  zn_vec3_to_array(origin, &origin_array);
  zn_vec3_to_array(direction, &direction_array);

  struct zgnr_seat_ray* seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zgn_ray_send_enter(seat_ray->resource, serial, virtual_object->resource,
          &origin_array, &direction_array);
    }
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

void
zgnr_seat_send_ray_motion(struct zgnr_seat* parent, struct wl_client* client,
    uint32_t time_msec, vec3 origin, vec3 direction)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);

  struct wl_array origin_array;
  struct wl_array direction_array;
  zn_vec3_to_array(origin, &origin_array);
  zn_vec3_to_array(direction, &direction_array);

  struct zgnr_seat_ray* seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zgn_ray_send_motion(
          seat_ray->resource, time_msec, &origin_array, &direction_array);
    }
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

void
zgnr_seat_send_ray_leave(
    struct zgnr_seat* parent, struct zgnr_virtual_object* virtual_object)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);
  struct wl_client* client = wl_resource_get_client(virtual_object->resource);
  struct wl_display* display = wl_global_get_display(self->global);

  uint32_t serial = wl_display_next_serial(display);

  struct zgnr_seat_ray* seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zgn_ray_send_leave(seat_ray->resource, serial, virtual_object->resource);
    }
  }
}

static void
zgnr_seat_handle_destroy(struct wl_resource* resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zgnr_seat_protocol_get_ray(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zgnr_seat_impl* self = wl_resource_get_user_data(resource);
  (void)zgnr_seat_ray_create(client, id, self);
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

  wl_resource_set_implementation(
      resource, &implementation, self, zgnr_seat_handle_destroy);

  wl_list_insert(&self->resource_list, wl_resource_get_link(resource));

  zgn_seat_send_capabilities(resource, self->base.capabilities);
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

  wl_list_init(&self->resource_list);
  wl_list_init(&self->seat_ray_list);

  return &self->base;

err:
  return NULL;
}

void
zgnr_seat_destroy(struct zgnr_seat* parent)
{
  struct zgnr_seat_impl* self = zn_container_of(parent, self, base);

  wl_global_destroy(self->global);

  wl_list_remove(&self->resource_list);
  wl_list_remove(&self->seat_ray_list);

  free(self);
}
