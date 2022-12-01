#include "zen/input/input-manager.h"

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

#include "zen-common.h"
#include "zen/binding.h"
#include "zen/input/input-device.h"
#include "zen/input/seat.h"

bool
zn_input_manager_bindings_notify_key(struct zn_input_manager *self,
    uint32_t time_msec, uint32_t key, enum wl_keyboard_key_state state,
    struct zn_keyboard *keyboard)
{
  struct zn_binding *binding, *tmp;

  wl_list_for_each_safe (binding, tmp, &self->key_binding_list, link) {
    if (zn_binding_notify_key(binding, time_msec, key, state, keyboard)) {
      return true;
    }
  }

  return false;
}

struct zn_binding *
zn_input_manager_add_key_binding(struct zn_input_manager *self, uint32_t key,
    uint32_t modifiers, zn_key_binding_handler_t handler, void *data)
{
  struct zn_binding *binding;

  binding = zn_binding_create(key, modifiers, handler, data);
  if (binding == NULL) {
    zn_error("Failed to create a key binding");
    return NULL;
  }

  wl_list_insert(&self->key_binding_list, &binding->link);

  return binding;
}

void
zn_input_manager_handle_new_wlr_input(
    struct zn_input_manager *self, struct wlr_input_device *wlr_input)
{
  struct zn_input_device *input_device =
      zn_input_device_create(self->seat, wlr_input);
  if (input_device == NULL) {
    zn_error("Failed to create zn_input_device");
    return;
  }

  // TODO: add multi seat support
}

struct zn_input_manager *
zn_input_manager_create(struct wl_display *display)
{
  struct zn_input_manager *self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->seat = zn_seat_create(display, ZEN_DEFAULT_SEAT);
  if (self->seat == NULL) {
    zn_error("Failed to create zn_seat");
    goto err_free;
  }

  wl_list_init(&self->key_binding_list);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_input_manager_destroy(struct zn_input_manager *self)
{
  struct zn_binding *binding, *tmp;

  wl_list_for_each_safe (binding, tmp, &self->key_binding_list, link) {
    zn_binding_destroy(binding);
  }

  zn_seat_destroy(self->seat);
  free(self);
}
