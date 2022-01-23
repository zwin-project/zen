#include "keyboard-client.h"

#include <libzen-compositor/libzen-compositor.h>
#include <zigen-server-protocol.h>

static void zen_keyboard_client_destroy(
    struct zen_keyboard_client *keyboard_client);

static void
zen_keyboard_client_handle_destroy(struct wl_resource *resource)
{
  struct zen_keyboard_client *keyboard_client;

  keyboard_client = wl_resource_get_user_data(resource);

  wl_list_remove(wl_resource_get_link(resource));

  if (keyboard_client && wl_list_empty(&keyboard_client->resource_list)) {
    zen_keyboard_client_destroy(keyboard_client);
  }
}

static void
zen_keyboard_client_protocol_release(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static const struct zgn_keyboard_interface keyboard_interface = {
    .release = zen_keyboard_client_protocol_release,
};

static void
zen_keyboard_client_keyboard_destroy_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_keyboard_client *keyboard_client;

  keyboard_client =
      wl_container_of(listener, keyboard_client, keyboard_destroy_listener);

  zen_keyboard_client_destroy(keyboard_client);
}

static struct zen_keyboard_client *
zen_keyboard_client_create(
    struct wl_client *client, struct zen_keyboard *keyboard)
{
  struct zen_keyboard_client *keyboard_client;

  keyboard_client = zalloc(sizeof *keyboard_client);
  if (keyboard_client == NULL) {
    wl_client_post_no_memory(client);
    zen_log("keyboard client: failed to allocate memory\n");
    goto err;
  }

  keyboard_client->keyboard = keyboard;
  wl_list_insert(&keyboard->keyboard_client_list, &keyboard_client->link);
  keyboard_client->keyboard_destroy_listener.notify =
      zen_keyboard_client_keyboard_destroy_handler;
  wl_signal_add(
      &keyboard->destroy_signal, &keyboard_client->keyboard_destroy_listener);
  keyboard_client->client = client;
  wl_list_init(&keyboard_client->resource_list);

  return keyboard_client;

err:
  return NULL;
}

static void
zen_keyboard_client_destroy(struct zen_keyboard_client *keyboard_client)
{
  struct wl_resource *resource, *tmp;
  wl_resource_for_each_safe(resource, tmp, &keyboard_client->resource_list)
  {
    wl_resource_set_user_data(resource, NULL);
    wl_resource_set_destructor(resource, NULL);
    wl_list_init(wl_resource_get_link(resource));
  }

  wl_list_remove(&keyboard_client->keyboard_destroy_listener.link);
  wl_list_remove(&keyboard_client->link);
  free(keyboard_client);
}

WL_EXPORT int
zen_keyboard_client_add_resource(
    struct zen_keyboard_client *keyboard_client, uint32_t id)
{
  struct wl_resource *resource;

  resource = wl_resource_create(
      keyboard_client->client, &zgn_keyboard_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(keyboard_client->client);
    zen_log("keyboard client: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(resource, &keyboard_interface, keyboard_client,
      zen_keyboard_client_handle_destroy);

  wl_list_insert(
      &keyboard_client->resource_list, wl_resource_get_link(resource));

  // FIXME: send enter event if already focused

  return 0;

err:
  return -1;
}

WL_EXPORT struct zen_keyboard_client *
zen_keyboard_client_find(
    struct wl_client *client, struct zen_keyboard *keyboard)
{
  struct zen_keyboard_client *keyboard_client;

  wl_list_for_each(keyboard_client, &keyboard->keyboard_client_list, link)
  {
    if (keyboard_client->client == client) return keyboard_client;
  }

  return NULL;
}

WL_EXPORT struct zen_keyboard_client *
zen_keyboard_client_ensure(
    struct wl_client *client, struct zen_keyboard *keyboard)
{
  struct zen_keyboard_client *keyboard_client;

  keyboard_client = zen_keyboard_client_find(client, keyboard);

  if (keyboard_client) {
    return keyboard_client;
  } else {
    return zen_keyboard_client_create(client, keyboard);
  }
}

WL_EXPORT struct wl_resource *
zen_keyboard_client_create_inert_resource(struct wl_client *client, uint32_t id)
{
  struct wl_resource *resource;

  resource = wl_resource_create(client, &zgn_keyboard_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("keyboard client: failed to create a resource\n");
    goto err;
  }

  wl_resource_set_implementation(resource, &keyboard_interface, NULL, NULL);

  return resource;

err:
  return NULL;
}
