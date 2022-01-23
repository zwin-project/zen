#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "keyboard-client.h"
#include "ray-client.h"

static enum zgn_seat_capability
zen_seat_get_current_capabilities(struct zen_seat* seat)
{
  enum zgn_seat_capability caps = 0;

  if (seat->ray_device_count > 0) caps |= ZGN_SEAT_CAPABILITY_RAY;

  if (seat->keyboard_device_count > 0) caps |= ZGN_SEAT_CAPABILITY_KEYBOARD;

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
  struct zen_seat* seat;
  struct zen_keyboard_client* keyboard_client;

  seat = wl_resource_get_user_data(resource);

  if (seat && seat->keyboard) {
    keyboard_client = zen_keyboard_client_ensure(client, seat->keyboard);
    zen_keyboard_client_add_resource(keyboard_client, id);

    zen_keyboard_keymap(seat->keyboard, client);
  } else {
    zen_keyboard_client_create_inert_resource(client, id);
  }
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
  struct zen_keyboard* keyboard;

  if (seat->keyboard) {
    seat->keyboard_device_count += 1;
    if (seat->keyboard_device_count == 1) {
      zen_seat_send_updated_caps(seat);
    }
    return;
  }

  keyboard = zen_keyboard_create(seat);
  if (keyboard == NULL) {
    zen_log("seat: failed to create a keyboard\n");
    return;
  }

  seat->keyboard = keyboard;
  seat->keyboard_device_count = 1;

  zen_seat_send_updated_caps(seat);
}

WL_EXPORT void
zen_seat_notify_release_keyboard(struct zen_seat* seat)
{
  seat->keyboard_device_count--;
  if (seat->keyboard_device_count == 0) {
    zen_seat_send_updated_caps(seat);
  }
}

WL_EXPORT void
zen_seat_notify_ray_motion(struct zen_seat* seat, const struct timespec* time,
    struct zen_ray_motion_event* event)
{
  struct zen_ray* ray = seat->ray;
  if (ray) {
    ray->grab->interface->motion(ray->grab, time, event);
  }
}

WL_EXPORT void
zen_seat_notify_ray_button(struct zen_seat* seat, const struct timespec* time,
    int32_t button, enum zgn_ray_button_state state)
{
  struct zen_ray* ray = seat->ray;
  if (ray == NULL) return;

  if (state == ZGN_RAY_BUTTON_STATE_PRESSED)
    ray->button_count++;
  else
    ray->button_count--;

  ray->grab->interface->button(ray->grab, time, button, state);

  if (ray->button_count == 1)
    ray->grab_serial = wl_display_get_serial(ray->seat->compositor->display);
}

WL_EXPORT void
zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, enum zgn_keyboard_key_state state)
{
  struct zen_keyboard* keyboard = seat->keyboard;
  if (keyboard == NULL) return;

  keyboard->grab->interface->key(keyboard->grab, time, key, state);
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
  seat->keyboard = NULL;
  seat->keyboard_device_count = 0;
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
  if (seat->keyboard) zen_keyboard_destroy(seat->keyboard);
  wl_global_destroy(seat->global);
  free(seat);
}
