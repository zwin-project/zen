#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <xkbcommon/xkbcommon.h>
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

static void
zen_seat_notify_modifiers(struct zen_seat* seat, uint32_t serial)
{
  struct zen_keyboard* keyboard = seat->keyboard;
  uint32_t mods_depressed, mods_latched, mods_locked, group;
  bool changed = false;

  mods_depressed =
      xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_DEPRESSED);
  mods_latched =
      xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_LATCHED);
  mods_locked =
      xkb_state_serialize_mods(keyboard->state, XKB_STATE_MODS_LOCKED);
  group =
      xkb_state_serialize_layout(keyboard->state, XKB_STATE_LAYOUT_EFFECTIVE);

  if (mods_depressed != keyboard->modifiers.mods_depressed ||
      mods_latched != keyboard->modifiers.mods_latched ||
      mods_locked != keyboard->modifiers.mods_locked ||
      group != keyboard->modifiers.group)
    changed = true;

  keyboard->modifiers.mods_depressed = mods_depressed;
  keyboard->modifiers.mods_latched = mods_latched;
  keyboard->modifiers.mods_locked = mods_locked;
  keyboard->modifiers.group = group;

  if (changed)
    keyboard->grab->interface->modifiers(keyboard->grab, serial,
        keyboard->modifiers.mods_depressed, keyboard->modifiers.mods_latched,
        keyboard->modifiers.mods_locked, keyboard->modifiers.group);
}

static void
zen_seat_update_modifier_state(struct zen_seat* seat, uint32_t serial,
    uint32_t key, enum zgn_keyboard_key_state state)
{
  struct zen_keyboard* keyboard = seat->keyboard;
  enum xkb_key_direction direction;

  if (state == ZGN_KEYBOARD_KEY_STATE_PRESSED)
    direction = XKB_KEY_DOWN;
  else
    direction = XKB_KEY_UP;

  /* Offset the keycode by 8, as the evdev XKB rules refrect X's broken keycode
   * system, which starts at 8*/
  xkb_state_update_key(keyboard->state, key + 8, direction);

  zen_seat_notify_modifiers(seat, serial);
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
    if (seat->keyboard) zen_keyboard_destroy(seat->keyboard);
    seat->keyboard = NULL;
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

  if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
    if (ray->button_count == 0) {
      ray->grab_button = button;
    }
    ray->button_count++;
  } else
    ray->button_count--;

  ray->grab->interface->button(ray->grab, time, button, state);

  if (ray->button_count == 1)
    ray->grab_serial = wl_display_get_serial(ray->seat->compositor->display);
}

WL_EXPORT void
zen_seat_notify_ray_axis(struct zen_seat* seat, const struct timespec* time,
    struct zen_ray_axis_event* event)
{
  struct zen_ray* ray = seat->ray;
  if (ray == NULL) return;

  ray->grab->interface->axis(ray->grab, time, event);
}

WL_EXPORT void
zen_seat_notify_ray_frame(struct zen_seat* seat)
{
  struct zen_ray* ray = seat->ray;
  if (ray == NULL) return;

  ray->grab->interface->frame(ray->grab);
}

WL_EXPORT void
zen_seat_notify_key(struct zen_seat* seat, const struct timespec* time,
    uint32_t key, enum zgn_keyboard_key_state state)
{
  struct zen_keyboard* keyboard = seat->keyboard;
  if (keyboard == NULL) return;

  keyboard->grab->interface->key(keyboard->grab, time, key, state);

  zen_seat_update_modifier_state(
      seat, wl_display_get_serial(seat->compositor->display), key, state);
}

WL_EXPORT struct zen_seat*
zen_seat_create(struct zen_compositor* compositor)
{
  struct zen_seat* seat;
  struct zen_data_device* data_device;
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

  data_device = zen_data_device_create(seat);
  if (data_device == NULL) {
    zen_log("seat: failed to create a data device\n");
    goto err_data_device;
  }
  seat->data_device = data_device;

  seat->compositor = compositor;

  seat->ray = NULL;
  seat->ray_device_count = 0;
  seat->keyboard = NULL;
  seat->keyboard_device_count = 0;
  wl_list_init(&seat->resource_list);
  seat->seat_name = compositor->config->seat;

  return seat;

err_data_device:
  wl_global_destroy(global);

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

  zen_data_device_destroy(seat->data_device);
  if (seat->ray) zen_ray_destroy(seat->ray);
  if (seat->keyboard) zen_keyboard_destroy(seat->keyboard);
  wl_global_destroy(seat->global);
  free(seat);
}
