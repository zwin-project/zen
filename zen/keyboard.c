#include "zen/keyboard.h"

#include <linux/input.h>
#include <wlr/types/wlr_seat.h>

#include "zen-common.h"
#include "zen/server.h"

static void
zn_keyboard_handle_key(struct wl_listener* listener, void* data)
{
  struct zn_server* server = zn_server_get_singleton();
  struct wlr_event_keyboard_key* event = data;
  UNUSED(listener);

  // Terminate the program with a keyboard event for development convenience.
  if (event->keycode == KEY_Q && event->state == WL_KEYBOARD_KEY_STATE_PRESSED)
    zn_server_terminate(server, EXIT_SUCCESS);
}

struct zn_keyboard*
zn_keyboard_create(struct wlr_input_device* wlr_input_device)
{
  if (!zn_assert(wlr_input_device->type == WLR_INPUT_DEVICE_KEYBOARD,
          "Wrong type - expect: %d, actual: %d", WLR_INPUT_DEVICE_KEYBOARD,
          wlr_input_device->type)) {
    goto err;
  }

  struct zn_keyboard* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->key_listener.notify = zn_keyboard_handle_key;
  wl_signal_add(&wlr_input_device->keyboard->events.key, &self->key_listener);

  return self;

err:
  return NULL;
}

void
zn_keyboard_destroy(struct zn_keyboard* self)
{
  wl_list_remove(&self->key_listener.link);
  free(self);
}
