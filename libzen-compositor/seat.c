#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "ray-client.h"

static enum zgn_seat_capability
zen_seat_get_current_capabilities(struct zen_seat* seat)
{
  enum zgn_seat_capability caps = 0;

  if (seat->ray_device_count > 0) caps |= ZGN_SEAT_CAPABILITY_RAY;

  return caps;
}

static void
zen_seat_send_updated_caps(struct zen_seat* seat)
{
  struct wl_resource* resource;

  enum zgn_seat_capability caps = zen_seat_get_current_capabilities(seat);

  wl_resource_for_each(resource, &seat->resource_list)
      zgn_seat_send_capabilities(resource, caps);
}

static void
zen_seat_protocol_get_ray(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  struct zen_seat* seat;
  struct zen_ray_client* ray_client;

  seat = wl_resource_get_user_data(resource);

  if (seat && seat->ray) {
    ray_client = zen_ray_client_ensure(client, seat->ray);
    zen_ray_client_add_resource(ray_client, id);
  } else {
    zen_ray_client_create_inert_resource(client, id);
  }
}

static void
zen_seat_protocol_get_keyboard(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
}

static void
zen_seat_protocol_release(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  UNUSED(resource);
}

static const struct zgn_seat_interface seat_interface = {
    .get_ray = zen_seat_protocol_get_ray,
    .get_keyboard = zen_seat_protocol_get_keyboard,
    .release = zen_seat_protocol_release,
};

static void
unbind_resource(struct wl_resource* resource)
{
  wl_list_remove(wl_resource_get_link(resource));
}

static void
zen_seat_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zen_seat* seat = data;
  struct wl_resource* resource;
  enum zgn_seat_capability caps;

  resource = wl_resource_create(client, &zgn_seat_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("seat: failed to create a resource\n");
    return;
  }

  wl_list_insert(&seat->resource_list, wl_resource_get_link(resource));

  wl_resource_set_implementation(
      resource, &seat_interface, seat, unbind_resource);

  caps = zen_seat_get_current_capabilities(seat);

  zgn_seat_send_capabilities(resource, caps);
}

WL_EXPORT void
zen_seat_notify_add_ray(struct zen_seat* seat)
{
  UNUSED(seat);
  struct zen_ray* ray;

  if (seat->ray) {
    seat->ray_device_count += 1;
    if (seat->ray_device_count == 1) {
      zen_seat_send_updated_caps(seat);
    }
    return;
  }

  ray = zen_ray_create(seat);
  if (ray == NULL) {
    zen_log("seat: failed to create a ray\n");
    return;
  }

  seat->ray = ray;
  seat->ray_device_count = 1;

  zen_seat_send_updated_caps(seat);
}

WL_EXPORT void
zen_seat_notify_release_ray(struct zen_seat* seat)
{
  seat->ray_device_count--;
  if (seat->ray_device_count == 0) {
    zen_seat_send_updated_caps(seat);
  }
}

WL_EXPORT void
zen_seat_notify_add_keyboard(struct zen_seat* seat)
{
  UNUSED(seat);
}

WL_EXPORT void
zen_seat_notify_release_keyboard(struct zen_seat* seat)
{
  UNUSED(seat);
}

WL_EXPORT void
zen_seat_notify_ray_motion(struct zen_seat* seat, const struct timespec* time,
    struct zen_pointer_motion_event* event)
{
  UNUSED(time);
  if (seat->ray) {
    // FIXME: use grab
    glm_vec3_add(seat->ray->origin, event->delta_origin, seat->ray->origin);
    seat->ray->angle.polar += event->delta_polar_angle;
    while (seat->ray->angle.polar >= GLM_PI * 2)
      seat->ray->angle.polar -= GLM_PI * 2;
    while (seat->ray->angle.polar < 0) seat->ray->angle.polar += GLM_PI * 2;

    seat->ray->angle.azimuthal += event->delta_azimuthal_angle;
    while (seat->ray->angle.azimuthal >= GLM_PI * 2)
      seat->ray->angle.azimuthal -= GLM_PI * 2;
    while (seat->ray->angle.azimuthal < 0)
      seat->ray->angle.azimuthal += GLM_PI * 2;

    seat->ray->render_item->commit(seat->ray->render_item);
  }
}

WL_EXPORT void
zen_seat_notify_ray_button(struct zen_seat* seat, const struct timespec* time,
    int32_t button, enum wl_pointer_button_state state)
{
  UNUSED(seat);
  UNUSED(time);
  UNUSED(button);
  UNUSED(state);
}

WL_EXPORT void
zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, enum zgn_keyboard_key_state state)
{
  UNUSED(seat);
  UNUSED(time);
  UNUSED(key);
  UNUSED(state);
}

WL_EXPORT struct zen_seat*
zen_seat_create(struct zen_compositor* compositor)
{
  struct zen_seat* seat;
  struct wl_global* global;

  seat = zalloc(sizeof *seat);
  if (seat == NULL) {
    zen_log("seat: failed to allocate memroy\n");
    goto err;
  }

  global = wl_global_create(
      compositor->display, &zgn_seat_interface, 1, seat, zen_seat_bind);
  if (global == NULL) {
    zen_log("seat: failed to create a seat global\n");
    goto err_global;
  }

  seat->global = global;
  seat->compositor = compositor;
  seat->ray = NULL;
  seat->ray_device_count = 0;
  wl_list_init(&seat->resource_list);
  seat->seat_name = "seat0";

  return seat;

err_global:
  free(seat);

err:
  return NULL;
}

WL_EXPORT void
zen_seat_destroy(struct zen_seat* seat)
{
  struct wl_resource *resource, *tmp;

  wl_resource_for_each_safe(resource, tmp, &seat->resource_list)
      wl_resource_set_user_data(resource, NULL);

  wl_list_remove(&seat->resource_list);

  if (seat->ray) zen_ray_destroy(seat->ray);
  wl_global_destroy(seat->global);
  free(seat);
}
