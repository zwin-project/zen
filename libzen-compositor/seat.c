#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

static void
zen_seat_protocol_get_ray(
    struct wl_client* client, struct wl_resource* resource, uint32_t id)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(id);
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
zen_seat_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zen_seat* seat = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &zgn_seat_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("seat: failed to create a resource\n");
  }

  wl_resource_set_implementation(resource, &seat_interface, seat, NULL);
}

WL_EXPORT void
zen_seat_notify_add_ray(struct zen_seat* seat)
{
  UNUSED(seat);
  zen_log("ray added\n");
}

WL_EXPORT void
zen_seat_notify_release_ray(struct zen_seat* seat)
{
  UNUSED(seat);
  zen_log("ray released\n");
}

WL_EXPORT void
zen_seat_notify_add_keyboard(struct zen_seat* seat)
{
  UNUSED(seat);
  zen_log("key board added\n");
}

WL_EXPORT void
zen_seat_notify_release_keyboard(struct zen_seat* seat)
{
  zen_log("keyboard released\n");
  UNUSED(seat);
}

WL_EXPORT void
zen_seat_notify_ray_motion(struct zen_seat* seat, const struct timespec* time,
    struct zen_pointer_motion_event* event)
{
  UNUSED(seat);
  UNUSED(time);
  UNUSED(event);
  zen_log("motion theta = %f, phi = %f\n", event->delta_azimuthal_angle,
      event->delta_polar_angle);
}

WL_EXPORT void
zen_seat_notify_ray_button(struct zen_seat* seat, const struct timespec* time,
    int32_t button, enum wl_pointer_button_state state)
{
  UNUSED(seat);
  UNUSED(time);
  UNUSED(button);
  UNUSED(state);
  zen_log("pointer button\n");
}

WL_EXPORT void
zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, enum zgn_keyboard_key_state state)
{
  UNUSED(seat);
  UNUSED(time);
  if (state == ZGN_KEYBOARD_KEY_STATE_PRESSED) {
    zen_log("%d is pressed\n", key);
  } else {
    zen_log("%d is released\n", key);
  }
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
  wl_global_destroy(seat->global);
  free(seat);
}
