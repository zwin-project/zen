#ifndef ZEN_INPUT_MANAGER_H
#define ZEN_INPUT_MANAGER_H

#include <wayland-server.h>
#include <wlr/types/wlr_input_device.h>

#include "zen/binding.h"
#include "zen/input-device.h"
#include "zen/keyboard.h"

struct zn_input_manager {
  struct zn_seat* seat;
  struct wl_list key_binding_list;  // zn_binding::link
};

/**
 * @return true if the key event is handled as the key binding
 */
bool zn_input_manager_bindings_notify_key(struct zn_input_manager* self,
    uint32_t time_msec, uint32_t key, enum wl_keyboard_key_state state,
    struct zn_keyboard* keyboard);

/**
 * @return NULL if failed
 */
struct zn_binding* zn_input_manager_add_key_binding(
    struct zn_input_manager* self, uint32_t key, uint32_t modifiers,
    zn_key_binding_handler_t handler, void* data);

void zn_input_manager_handle_new_wlr_input(
    struct zn_input_manager* self, struct wlr_input_device* wlr_input);

struct zn_input_manager* zn_input_manager_create(struct wl_display* display);

void zn_input_manager_destroy(struct zn_input_manager* self);

#endif  // ZEN_INPUT_MANAGER_H
