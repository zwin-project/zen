#ifndef ZEN_BINDING_H
#define ZEN_BINDING_H

#include <wayland-server.h>

struct zn_input_manager;
struct zn_keyboard;

typedef void (*zn_key_binding_handler_t)(
    uint32_t time_msec, uint32_t key, void *data);

struct zn_binding {
  uint32_t key;
  uint32_t modifiers;
  zn_key_binding_handler_t handler;
  void *data;
  struct wl_list link;  // zn_input_manager::key_binding_list
};

/**
 * @return true if the key event is handled
 */
bool zn_binding_notify_key(struct zn_binding *self, uint32_t time_msec,
    uint32_t key, enum wl_keyboard_key_state state,
    struct zn_keyboard *keyboard);

struct zn_binding *zn_binding_create(uint32_t key, uint32_t modifiers,
    zn_key_binding_handler_t handler, void *data);

void zn_binding_destroy(struct zn_binding *self);

#endif  //  ZEN_BINDING_H
