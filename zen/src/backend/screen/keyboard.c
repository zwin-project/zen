#include "keyboard.h"

#include <wlr/types/wlr_keyboard.h>

#include "default-backend.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"

static void zn_keyboard_destroy(struct zn_keyboard *self);

static void
zn_keyboard_handle_wlr_input_device_destroy(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_keyboard *self =
      zn_container_of(listener, self, wlr_input_device_destroy_listener);

  zn_keyboard_destroy(self);
}

static void
zn_keyboard_handle_keyboard_key(struct wl_listener *listener, void *user_data)
{
  struct zn_keyboard *self =
      zn_container_of(listener, self, keyboard_key_listener);
  struct wlr_event_keyboard_key *event = user_data;
  struct zn_server *server = zn_server_get_singleton();

  // TODO(@Aki-7): Handle keyboard bindings

  wlr_seat_set_keyboard(server->seat->wlr_seat, self->base.wlr_input_device);
  wlr_seat_keyboard_send_key(
      server->seat->wlr_seat, event->time_msec, event->keycode, event->state);
}

static void
zn_keyboard_handle_keyboard_modifiers(
    struct wl_listener *listener, void *user_data UNUSED)
{
  struct zn_keyboard *self =
      zn_container_of(listener, self, keyboard_modifiers_listener);
  struct zn_server *server = zn_server_get_singleton();

  wlr_seat_set_keyboard(server->seat->wlr_seat, self->base.wlr_input_device);
  wlr_seat_keyboard_send_modifiers(server->seat->wlr_seat,
      &self->base.wlr_input_device->keyboard->modifiers);
}

struct zn_keyboard *
zn_keyboard_create(struct wlr_input_device *wlr_input_device)
{
  struct zn_keyboard *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.wlr_input_device = wlr_input_device;
  wl_list_init(&self->base.link);

  self->wlr_input_device_destroy_listener.notify =
      zn_keyboard_handle_wlr_input_device_destroy;
  wl_signal_add(&self->base.wlr_input_device->events.destroy,
      &self->wlr_input_device_destroy_listener);

  self->keyboard_key_listener.notify = zn_keyboard_handle_keyboard_key;
  wl_signal_add(
      &wlr_input_device->keyboard->events.key, &self->keyboard_key_listener);

  self->keyboard_modifiers_listener.notify =
      zn_keyboard_handle_keyboard_modifiers;
  wl_signal_add(&wlr_input_device->keyboard->events.modifiers,
      &self->keyboard_modifiers_listener);

  {
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap *keymap =
        xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);
    wlr_keyboard_set_keymap(wlr_input_device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
  }

  wlr_keyboard_set_repeat_info(wlr_input_device->keyboard, 25, 600);

  struct zn_server *server = zn_server_get_singleton();
  wlr_seat_set_keyboard(server->seat->wlr_seat, wlr_input_device);

  return self;

err:
  return NULL;
}

static void
zn_keyboard_destroy(struct zn_keyboard *self)
{
  wl_list_remove(&self->wlr_input_device_destroy_listener.link);
  wl_list_remove(&self->keyboard_key_listener.link);
  wl_list_remove(&self->keyboard_modifiers_listener.link);
  wl_list_remove(&self->base.link);
  free(self);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);
  zn_default_backend_update_capabilities(backend);
}
