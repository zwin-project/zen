#include "zen/keyboard.h"

#include <wlr/types/wlr_seat.h>

#include "zen-common.h"

static void
handle_key(struct wl_listener* listener, void* data)
{
  UNUSED(listener);
  UNUSED(data);
  exit(0);
}

struct zn_keyboard*
zn_keyboard_create(
    struct zn_seat* seat, struct wlr_input_device* wlr_input_device)
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

  self->key_listener.notify = handle_key;
  wl_signal_add(&wlr_input_device->keyboard->events.key, &self->key_listener);
  wl_list_insert(&seat->keyboards, &self->link);

  return self;

err:
  return NULL;
}

void
zn_keyboard_destroy(struct zn_keyboard* self)
{
  wl_list_remove(&self->key_listener.link);
  wl_list_remove(&self->link);
  free(self);
}
