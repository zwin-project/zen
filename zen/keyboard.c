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

void
zn_keyboard_configure(
    struct zn_keyboard* self, struct zn_input_device* input_device)
{
  self->key_listener.notify = handle_key;
  wl_signal_add(
      &input_device->wlr_input->keyboard->events.key, &self->key_listener);
}

struct zn_keyboard*
zn_keyboard_create()
{
  struct zn_keyboard* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  return self;

err:
  return NULL;
}

void
zn_keyboard_destroy(struct zn_keyboard* self)
{
  free(self);
}
