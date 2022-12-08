#include "zen/input/binding.h"

#include <zen-common.h>

#include "zen/input/input-manager.h"
#include "zen/server.h"

bool
zn_binding_notify_key(struct zn_binding *self, uint32_t time_msec, uint32_t key,
    enum wl_keyboard_key_state state, struct zn_keyboard *keyboard)
{
  struct wlr_keyboard *wlr_keyboard =
      keyboard->input_device->wlr_input->keyboard;

  if (state == WL_KEYBOARD_KEY_STATE_RELEASED) {
    return false;
  }

  if (self->key == key &&
      self->modifiers == wlr_keyboard_get_modifiers(wlr_keyboard)) {
    self->handler(time_msec, key, self->data);
    return true;
  }

  return false;
}

struct zn_binding *
zn_binding_create(uint32_t key, uint32_t modifiers,
    zn_key_binding_handler_t handler, void *data)
{
  struct zn_binding *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->key = key;
  self->modifiers = modifiers;
  self->handler = handler;
  self->data = data;
  wl_list_init(&self->link);

  return self;

err:
  return NULL;
}

void
zn_binding_destroy(struct zn_binding *self)
{
  wl_list_remove(&self->link);
  free(self);
}
