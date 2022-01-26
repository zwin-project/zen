#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <unistd.h>
#include <xkbcommon/xkbcommon.h>
#include <zigen-server-protocol.h>

#include "keyboard-client.h"
#include "virtual-object.h"

static void
zen_keyboard_enter(
    struct zen_keyboard* keyboard, struct zen_virtual_object* virtual_object)
{
  struct zen_keyboard_client* keyboard_client;
  struct wl_resource* resource;
  uint32_t serial;

  keyboard_client = zen_keyboard_client_find(
      wl_resource_get_client(virtual_object->resource), keyboard);
  if (keyboard_client == NULL) return;

  serial = wl_display_next_serial(virtual_object->compositor->display);
  wl_resource_for_each(resource, &keyboard_client->resource_list)
  {
    zgn_keyboard_send_enter(
        resource, serial, virtual_object->resource, &keyboard->keys);
  }
}

static void
zen_keyboard_leave(struct zen_keyboard* keyboard)
{
  struct zen_virtual_object* virtual_object;
  struct zen_keyboard_client* keyboard_client;
  struct wl_resource* resource;
  uint32_t serial;

  virtual_object =
      zen_weak_link_get_user_data(&keyboard->focus_virtual_object_link);
  if (virtual_object == NULL) return;

  keyboard_client = zen_keyboard_client_find(
      wl_resource_get_client(virtual_object->resource), keyboard);
  if (keyboard_client == NULL) return;

  serial = wl_display_next_serial(virtual_object->compositor->display);
  wl_resource_for_each(resource, &keyboard_client->resource_list)
  {
    zgn_keyboard_send_leave(resource, serial, virtual_object->resource);
  }
}

static void
default_grab_key(struct zen_keyboard_grab* grab, const struct timespec* time,
    uint32_t key, uint32_t state)
{
  struct zen_keyboard* keyboard = grab->keyboard;
  struct zen_virtual_object* focus_virtual_object;
  struct zen_keyboard_client* keyboard_client;
  struct wl_resource* resource;
  uint32_t serial, msec;

  focus_virtual_object =
      zen_weak_link_get_user_data(&keyboard->focus_virtual_object_link);
  if (focus_virtual_object == NULL) return;

  keyboard_client = zen_keyboard_client_find(
      wl_resource_get_client(focus_virtual_object->resource), keyboard);
  if (keyboard_client == NULL) return;

  msec = timespec_to_msec(time);

  serial = wl_display_next_serial(keyboard->seat->compositor->display);
  wl_resource_for_each(resource, &keyboard_client->resource_list)
  {
    zgn_keyboard_send_key(resource, serial, msec, key, state);
    wl_client_flush(wl_resource_get_client(resource));
  }
}

static void
default_grab_modifiers(struct zen_keyboard_grab* grab, uint32_t serial,
    uint32_t mods_depressed, uint32_t mods_latched, uint32_t mods_locked,
    uint32_t group)
{
  UNUSED(grab);
  UNUSED(serial);
  UNUSED(mods_depressed);
  UNUSED(mods_latched);
  UNUSED(mods_locked);
  UNUSED(group);
}

static void
default_grab_cancel(struct zen_keyboard_grab* grab)
{
  UNUSED(grab);
}

static const struct zen_keyboard_grab_interface
    default_keyboard_grab_interface = {
        .key = default_grab_key,
        .modifiers = default_grab_modifiers,
        .cancel = default_grab_cancel,
};

static int
zen_keyboard_add_keymap(struct zen_keyboard* keyboard)
{
  struct xkb_context* context;
  struct xkb_keymap* keymap;
  char* keymap_string;

  context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  if (context == NULL) {
    zen_log("keyboard: failed to create XKB context\n");
    goto out;
  }

  keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
  if (keymap == NULL) {
    zen_log("keyboard: failed to create XKB keymap\n");
    goto out_context;
  }

  keymap_string = xkb_keymap_get_as_string(keymap, XKB_KEYMAP_FORMAT_TEXT_V1);
  if (keymap_string == NULL) {
    zen_log("keyboard: failed to get XKB keymap as string\n");
    goto out_keymap;
  }

  keyboard->keymap_size = strlen(keymap_string) + 1;
  keyboard->keymap_format = ZGN_KEYBOARD_KEYMAP_FORMAT_XKB_V1;
  keyboard->keymap_fd =
      zen_util_create_shared_file(keyboard->keymap_size, keymap_string);
  if (keyboard->keymap_fd < 0) {
    zen_log("keyboard: failed to create a keymap file\n");
    goto out_file;
  }

  free(keymap_string);

  xkb_keymap_unref(keymap);

  xkb_context_unref(context);

  return 0;

out_file:
  free(keymap_string);

out_keymap:
  xkb_keymap_unref(keymap);

out_context:
  xkb_context_unref(context);

out:
  return -1;
}

WL_EXPORT void
zen_keyboard_set_focus(
    struct zen_keyboard* keyboard, struct zen_virtual_object* virtual_object)
{
  struct zen_virtual_object* current_focus;

  current_focus =
      zen_weak_link_get_user_data(&keyboard->focus_virtual_object_link);

  if (current_focus == virtual_object) return;

  if (current_focus) zen_keyboard_leave(keyboard);
  if (virtual_object) zen_keyboard_enter(keyboard, virtual_object);

  if (virtual_object)
    zen_weak_link_set(
        &keyboard->focus_virtual_object_link, virtual_object->resource);
  else
    zen_weak_link_unset(&keyboard->focus_virtual_object_link);
}

WL_EXPORT void
zen_keyboard_keymap(struct zen_keyboard* keyboard, struct wl_client* client)
{
  struct zen_keyboard_client* keyboard_client;
  struct wl_resource* resource;

  keyboard_client = zen_keyboard_client_find(client, keyboard);
  if (keyboard_client == NULL) return;

  wl_resource_for_each(resource, &keyboard_client->resource_list)
  {
    zgn_keyboard_send_keymap(resource, keyboard->keymap_format,
        keyboard->keymap_fd, keyboard->keymap_size);
  }
}

WL_EXPORT struct zen_keyboard*
zen_keyboard_create(struct zen_seat* seat)
{
  struct zen_keyboard* keyboard;
  int ret;

  keyboard = zalloc(sizeof *keyboard);
  if (keyboard == NULL) {
    zen_log("keyboard: failed to allocate memory\n");
    goto err;
  }

  ret = zen_keyboard_add_keymap(keyboard);
  if (ret < 0) {
    zen_log("keyboard: failed to add keymap\n");
    goto err_keymap;
  }
  keyboard->seat = seat;
  keyboard->grab = &keyboard->default_grab;
  keyboard->default_grab.interface = &default_keyboard_grab_interface;
  keyboard->default_grab.keyboard = keyboard;
  zen_weak_link_init(&keyboard->focus_virtual_object_link);
  wl_list_init(&keyboard->keyboard_client_list);
  wl_signal_init(&keyboard->destroy_signal);
  wl_array_init(&keyboard->keys);

  return keyboard;

err_keymap:
  free(keyboard);

err:
  return NULL;
}

WL_EXPORT void
zen_keyboard_destroy(struct zen_keyboard* keyboard)
{
  keyboard->grab->interface->cancel(keyboard->grab);
  zen_weak_link_unset(&keyboard->focus_virtual_object_link);
  wl_signal_emit(&keyboard->destroy_signal, NULL);
  close(keyboard->keymap_fd);
  wl_array_release(
      &keyboard->keys);  // TODO: release the contents of this array
  free(keyboard);
}
