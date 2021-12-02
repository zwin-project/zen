#include "cuboid-window.h"

#include <string.h>
#include <zigen-shell-server-protocol.h>

char *zen_cuboid_window_role = "zen_cuboid_window";

static void zen_cuboid_window_destroy(struct zen_cuboid_window *cuboid_window);

static void
zen_cuboid_window_handle_destroy(struct wl_resource *resource)
{
  struct zen_cuboid_window *cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  zen_cuboid_window_destroy(cuboid_window);
}

static void
zen_cuboid_window_protocol_destroy(
    struct wl_client *client, struct wl_resource *resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_cuboid_window_protocol_ack_configure(
    struct wl_client *client, struct wl_resource *resource, uint32_t serial)
{
  // TODO: apply configuration after receiving ack_configure
  UNUSED(client);
  UNUSED(serial);
  struct zen_cuboid_window *cuboid_window;

  cuboid_window = wl_resource_get_user_data(resource);

  cuboid_window->render_item->commit(cuboid_window->render_item);
}

static void
zen_cuboid_window_protocol_move(struct wl_client *client,
    struct wl_resource *resource, struct wl_resource *seat, uint32_t serial)
{
  UNUSED(client);
  UNUSED(resource);
  UNUSED(seat);
  UNUSED(serial);
}

static const struct zgn_cuboid_window_interface cuboid_window_interface = {
    .destroy = zen_cuboid_window_protocol_destroy,
    .ack_configure = zen_cuboid_window_protocol_ack_configure,
    .move = zen_cuboid_window_protocol_move,
};

static void
zen_cuboid_window_virtual_object_destroy_handler(
    struct wl_listener *listener, void *data)
{
  // Clients must not destroy a bound virtual object before a cuboid window. But
  // if it does so, compositor will just destroy the cuboid window as well.
  // Sending implementation error to clients brings error when terminating the
  // client

  UNUSED(data);
  struct zen_cuboid_window *cuboid_window;

  cuboid_window =
      wl_container_of(listener, cuboid_window, virtual_object_destroy_listener);

  wl_resource_destroy(cuboid_window->resource);
}

static void
zen_cuboid_window_virtual_object_render_commit_handler(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zen_cuboid_window *cuboid_window;

  cuboid_window = wl_container_of(
      listener, cuboid_window, virtual_object_render_commit_listener);

  cuboid_window->render_item->commit(cuboid_window->render_item);
}

WL_EXPORT
struct zen_cuboid_window *
zen_cuboid_window_create(struct wl_client *client, uint32_t id,
    struct wl_resource *shell_resource,
    struct zen_virtual_object *virtual_object)
{
  struct zen_cuboid_window *cuboid_window;
  struct wl_resource *resource;
  struct zen_render_item *render_item;
  struct zen_shell *shell = wl_resource_get_user_data(shell_resource);
  vec3 zero = GLM_VEC3_ZERO_INIT;

  if (virtual_object->role_object != NULL) {
    wl_resource_post_error(shell_resource, ZGN_SHELL_ERROR_ROLE,
        "given virtual object has another role");
    zen_log("cuboid window: given virtual object has another role\n");
    goto err;
  }

  if (virtual_object->role != NULL &&
      strcmp(virtual_object->role, zen_cuboid_window_role) != 0) {
    wl_resource_post_error(shell_resource, ZGN_SHELL_ERROR_ROLE,
        "given virtual object has been attached another type of role");
    zen_log(
        "cuboid window: given virtual object has been attached another type of "
        "role\n");
    goto err;
  }

  cuboid_window = zalloc(sizeof *cuboid_window);
  if (cuboid_window == NULL) {
    wl_client_post_no_memory(client);
    zen_log("cuboid window: failed to allocate memory\n");
    goto err;
  }

  render_item = zen_cuboid_window_render_item_create(
      shell->compositor->renderer, cuboid_window);
  if (render_item == NULL) {
    wl_client_post_no_memory(client);
    zen_log("cuboid window: failed to create a render item\n");
    goto err_render_item;
  }

  resource = wl_resource_create(client, &zgn_cuboid_window_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("cuboid window: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &cuboid_window_interface,
      cuboid_window, zen_cuboid_window_handle_destroy);

  if (virtual_object->role == NULL)
    virtual_object->role = strdup(zen_cuboid_window_role);
  virtual_object->role_object = cuboid_window;

  cuboid_window->shell = shell;
  cuboid_window->resource = resource;
  cuboid_window->virtual_object = virtual_object;
  glm_vec3_copy(zero, cuboid_window->half_size);

  wl_list_insert(&shell->cuboid_window_list, &cuboid_window->link);

  cuboid_window->virtual_object_destroy_listener.notify =
      zen_cuboid_window_virtual_object_destroy_handler;
  wl_signal_add(&virtual_object->destroy_signal,
      &cuboid_window->virtual_object_destroy_listener);

  cuboid_window->virtual_object_render_commit_listener.notify =
      zen_cuboid_window_virtual_object_render_commit_handler;
  wl_signal_add(&virtual_object->render_commit_signal,
      &cuboid_window->virtual_object_render_commit_listener);

  cuboid_window->render_item = render_item;

  return cuboid_window;

err_resource:
  zen_cuboid_window_render_item_destroy(render_item);

err_render_item:
  free(cuboid_window);

err:
  return NULL;
}

static void
zen_cuboid_window_destroy(struct zen_cuboid_window *cuboid_window)
{
  cuboid_window->virtual_object->role_object = NULL;
  wl_list_remove(&cuboid_window->virtual_object_destroy_listener.link);
  wl_list_remove(&cuboid_window->virtual_object_render_commit_listener.link);
  wl_list_remove(&cuboid_window->link);
  zen_cuboid_window_render_item_destroy(cuboid_window->render_item);
  free(cuboid_window);
}

WL_EXPORT void
zen_cuboid_window_configure(
    struct zen_cuboid_window *cuboid_window, vec3 half_size)
{
  struct wl_array half_size_array;

  // TODO: apply configuration after receiving ack_configure
  glm_vec3_copy(half_size, cuboid_window->half_size);

  uint32_t serial =
      wl_display_next_serial(cuboid_window->shell->compositor->display);

  wl_array_init(&half_size_array);

  glm_vec3_to_wl_array(cuboid_window->half_size, &half_size_array);

  zgn_cuboid_window_send_configure(
      cuboid_window->resource, serial, &half_size_array);

  wl_array_release(&half_size_array);
}
