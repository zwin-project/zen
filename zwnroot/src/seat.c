#include "seat.h"

#include <zen-common.h>
#include <zwin-protocol.h>

#include "seat-ray.h"
#include "virtual-object.h"

static void
zwnr_seat_send_ray_enter(struct zwnr_seat_impl *self,
    struct zwnr_virtual_object *virtual_object, vec3 origin, vec3 direction)
{
  struct wl_client *client = wl_resource_get_client(virtual_object->resource);
  uint32_t serial = wl_display_next_serial(self->display);

  struct wl_array origin_array;
  struct wl_array direction_array;
  zn_vec3_to_array(origin, &origin_array);
  zn_vec3_to_array(direction, &direction_array);

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zwn_ray_send_enter(seat_ray->resource, serial, virtual_object->resource,
          &origin_array, &direction_array);
      zwn_ray_send_frame(seat_ray->resource);
    }
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

static void
zwnr_seat_send_ray_leave(
    struct zwnr_seat_impl *self, struct zwnr_virtual_object *virtual_object)
{
  struct wl_client *client = wl_resource_get_client(virtual_object->resource);
  uint32_t serial = wl_display_next_serial(self->display);

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zwn_ray_send_leave(seat_ray->resource, serial, virtual_object->resource);
    }
  }
}

static void
zwnr_seat_set_ray_focus_virtual_object(
    struct zwnr_seat_impl *self, struct zwnr_virtual_object *virtual_object)
{
  if (self->base.ray_state.focus_virtual_object) {
    wl_list_remove(&self->ray_focus_virtual_object_listener.link);
    wl_list_init(&self->ray_focus_virtual_object_listener.link);
  }

  if (virtual_object) {
    wl_signal_add(&virtual_object->events.destroy,
        &self->ray_focus_virtual_object_listener);
  }

  self->base.ray_state.focus_virtual_object = virtual_object;
}

void
zwnr_seat_set_capabilities(struct zwnr_seat *parent, uint32_t capabilities)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct wl_resource *resource;

  self->base.capabilities = capabilities;
  wl_resource_for_each (resource, &self->resource_list) {
    zwn_seat_send_capabilities(resource, capabilities);
  }
}

void
zwnr_seat_ray_enter(struct zwnr_seat *parent,
    struct zwnr_virtual_object *virtual_object, vec3 origin, vec3 direction)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);

  if (self->base.ray_state.focus_virtual_object) {
    zwnr_seat_send_ray_leave(self, self->base.ray_state.focus_virtual_object);
  }

  zwnr_seat_set_ray_focus_virtual_object(self, virtual_object);

  zwnr_seat_send_ray_enter(self, virtual_object, origin, direction);
}

void
zwnr_seat_ray_send_motion(
    struct zwnr_seat *parent, uint32_t time_msec, vec3 origin, vec3 direction)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct zwnr_virtual_object *virtual_object =
      self->base.ray_state.focus_virtual_object;
  struct wl_client *client;

  if (!virtual_object) return;
  client = wl_resource_get_client(virtual_object->resource);

  struct wl_array origin_array;
  struct wl_array direction_array;
  zn_vec3_to_array(origin, &origin_array);
  zn_vec3_to_array(direction, &direction_array);

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zwn_ray_send_motion(
          seat_ray->resource, time_msec, &origin_array, &direction_array);
    }
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

void
zwnr_seat_ray_clear_focus(struct zwnr_seat *parent)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct zwnr_virtual_object *virtual_object =
      self->base.ray_state.focus_virtual_object;

  if (!virtual_object) return;

  zwnr_seat_set_ray_focus_virtual_object(self, NULL);

  zwnr_seat_send_ray_leave(self, virtual_object);
}

void
zwnr_seat_ray_send_button(struct zwnr_seat *parent, uint32_t time_msec,
    uint32_t button, enum zwn_ray_button_state state)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct zwnr_virtual_object *virtual_object =
      self->base.ray_state.focus_virtual_object;
  struct wl_client *client;
  uint32_t serial = wl_display_next_serial(self->display);

  if (!virtual_object) return;
  client = wl_resource_get_client(virtual_object->resource);

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zwn_ray_send_button(seat_ray->resource, serial, time_msec, button, state);
    }
  }

  self->base.ray_state.last_button_serial = serial;
}

void
zwnr_seat_ray_send_axis(struct zwnr_seat *parent, uint32_t time_msec,
    enum zwn_ray_axis axis, double value, int32_t value_discrete,
    enum zwn_ray_axis_source source)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct zwnr_virtual_object *virtual_object =
      self->base.ray_state.focus_virtual_object;
  struct wl_client *client;

  if (!virtual_object) return;
  client = wl_resource_get_client(virtual_object->resource);

  bool should_send_source = false;
  if (!self->base.ray_state.sent_axis_source) {
    self->base.ray_state.sent_axis_source = true;
    should_send_source = true;
  }

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client != wl_resource_get_client(seat_ray->resource)) {
      continue;
    }

    if (should_send_source) {
      zwn_ray_send_axis_source(seat_ray->resource, source);
    }

    if (value) {
      if (value_discrete) {
        zwn_ray_send_axis_discrete(seat_ray->resource, axis, value_discrete);
      }

      zwn_ray_send_axis(
          seat_ray->resource, time_msec, axis, wl_fixed_from_double(value));
    } else {
      zwn_ray_send_axis_stop(seat_ray->resource, time_msec, axis);
    }
  }
}

void
zwnr_seat_ray_send_frame(struct zwnr_seat *parent)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);
  struct zwnr_virtual_object *virtual_object =
      self->base.ray_state.focus_virtual_object;
  struct wl_client *client;

  if (!virtual_object) return;
  client = wl_resource_get_client(virtual_object->resource);

  self->base.ray_state.sent_axis_source = false;

  struct zwnr_seat_ray *seat_ray;
  wl_list_for_each (seat_ray, &self->seat_ray_list, link) {
    if (client == wl_resource_get_client(seat_ray->resource)) {
      zwn_ray_send_frame(seat_ray->resource);
    }
  }
}

static void
zwnr_seat_handle_destroy(struct wl_resource *resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zwnr_seat_protocol_get_ray(
    struct wl_client *client, struct wl_resource *resource, uint32_t id)
{
  struct zwnr_seat_impl *self = wl_resource_get_user_data(resource);
  (void)zwnr_seat_ray_create(client, id, self);
}

static void
zwnr_seat_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zwn_seat_interface implementation = {
    .get_ray = zwnr_seat_protocol_get_ray,
    .release = zwnr_seat_protocol_release,
};

static void
zwnr_seat_handle_ray_focus_virtual_object_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zwnr_seat_impl *self =
      zn_container_of(listener, self, ray_focus_virtual_object_listener);
  zwnr_seat_set_ray_focus_virtual_object(self, NULL);
}

static void
zwnr_seat_bind(
    struct wl_client *client, void *data, uint32_t version, uint32_t id)
{
  struct zwnr_seat_impl *self = data;

  struct wl_resource *resource =
      wl_resource_create(client, &zwn_seat_interface, version, id);
  if (resource == NULL) {
    zn_error("Failed to creat a wl_resource");
    wl_client_post_no_memory(client);
    return;
  }

  wl_resource_set_implementation(
      resource, &implementation, self, zwnr_seat_handle_destroy);

  wl_list_insert(&self->resource_list, wl_resource_get_link(resource));

  zwn_seat_send_capabilities(resource, self->base.capabilities);
}

struct zwnr_seat *
zwnr_seat_create(struct wl_display *display)
{
  struct zwnr_seat_impl *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->global =
      wl_global_create(display, &zwn_seat_interface, 1, self, zwnr_seat_bind);
  self->display = display;
  self->base.ray_state.sent_axis_source = false;

  wl_list_init(&self->resource_list);
  wl_list_init(&self->seat_ray_list);

  self->ray_focus_virtual_object_listener.notify =
      zwnr_seat_handle_ray_focus_virtual_object_destroy;
  wl_list_init(&self->ray_focus_virtual_object_listener.link);

  return &self->base;

err:
  return NULL;
}

void
zwnr_seat_destroy(struct zwnr_seat *parent)
{
  struct zwnr_seat_impl *self = zn_container_of(parent, self, base);

  wl_global_destroy(self->global);

  wl_list_remove(&self->resource_list);
  wl_list_remove(&self->seat_ray_list);
  wl_list_remove(&self->ray_focus_virtual_object_listener.link);

  free(self);
}
