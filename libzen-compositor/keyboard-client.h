#ifndef LIBZEN_COMPOSITOR_KEYBOARD_CLIENT_H
#define LIBZEN_COMPOSITOR_KEYBOARD_CLIENT_H

#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server-core.h>

struct zen_keyboard_client {
  struct zen_keyboard *keyboard;
  struct wl_list link;
  struct wl_listener keyboard_destroy_listener;

  struct wl_client *client;
  struct wl_list resource_list;
};

int zen_keyboard_client_add_resource(
    struct zen_keyboard_client *keyboard_client, uint32_t id);

struct zen_keyboard_client *zen_keyboard_client_find(
    struct wl_client *client, struct zen_keyboard *keyboard);

struct zen_keyboard_client *zen_keyboard_client_ensure(
    struct wl_client *client, struct zen_keyboard *keyboard);

struct wl_resource *zen_keyboard_client_create_insert_resource(
    struct wl_client *client, uint32_t id);

#endif  // LIBZEN_COMPOSITOR_KEYBOARD_CLIENT_H
