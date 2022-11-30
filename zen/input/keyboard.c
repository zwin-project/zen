#include "zen/input/keyboard.h"

#include <wlr/types/wlr_seat.h>
#include <xkbcommon/xkbcommon.h>

#include "zen-common.h"
#include "zen/server.h"

static void
zn_keyboard_handle_modifiers(struct wl_listener* listener, void* data)
{
  struct zn_keyboard* self =
      zn_container_of(listener, self, modifiers_listener);
  struct wlr_input_device* wlr_input = self->input_device->wlr_input;
  struct wlr_seat* wlr_seat = self->input_device->seat->wlr_seat;
  UNUSED(data);

  wlr_seat_set_keyboard(wlr_seat, wlr_input);
  wlr_seat_keyboard_notify_modifiers(wlr_seat, &wlr_input->keyboard->modifiers);
}

static void
zn_keyboard_handle_key(struct wl_listener* listener, void* data)
{
  struct zn_keyboard* self = zn_container_of(listener, self, key_listener);
  struct wlr_event_keyboard_key* event = data;
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_seat* wlr_seat = self->input_device->seat->wlr_seat;
  bool handled;

  handled = zn_input_manager_bindings_notify_key(server->input_manager,
      event->time_msec, event->keycode, event->state, self);

  if (!handled) {
    wlr_seat_set_keyboard(wlr_seat, self->input_device->wlr_input);
    wlr_seat_keyboard_notify_key(
        wlr_seat, event->time_msec, event->keycode, event->state);
  }
}

struct zn_keyboard*
zn_keyboard_create(struct zn_input_device* input_device, struct zn_seat* seat)
{
  struct zn_keyboard* self;
  struct wlr_input_device* wlr_input = input_device->wlr_input;

  if (!zn_assert(wlr_input->type == WLR_INPUT_DEVICE_KEYBOARD,
          "Wrong type - expect: %d, actual: %d", WLR_INPUT_DEVICE_KEYBOARD,
          wlr_input->type)) {
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->input_device = input_device;

  {
    struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap* keymap =
        xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_input->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
  }

  wlr_keyboard_set_repeat_info(wlr_input->keyboard, 25, 600);

  self->modifiers_listener.notify = zn_keyboard_handle_modifiers;
  wl_signal_add(
      &wlr_input->keyboard->events.modifiers, &self->modifiers_listener);

  self->key_listener.notify = zn_keyboard_handle_key;
  wl_signal_add(&wlr_input->keyboard->events.key, &self->key_listener);

  wlr_seat_set_keyboard(seat->wlr_seat, input_device->wlr_input);

  return self;

err:
  return NULL;
}

void
zn_keyboard_destroy(struct zn_keyboard* self)
{
  struct wlr_seat* wlr_seat = self->input_device->seat->wlr_seat;
  struct wlr_keyboard* wlr_keyboard = self->input_device->wlr_input->keyboard;

  if (wlr_seat_get_keyboard(wlr_seat) == wlr_keyboard) {
    wlr_seat_set_keyboard(wlr_seat, NULL);
  }

  wl_list_remove(&self->key_listener.link);
  wl_list_remove(&self->modifiers_listener.link);
  free(self);
}
