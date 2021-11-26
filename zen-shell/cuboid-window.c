#include "cuboid-window.h"

#include <zigen-shell-server-protocol.h>

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
  UNUSED(resource);
  UNUSED(serial);
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

WL_EXPORT struct zen_cuboid_window *
zen_cuboid_window_create(struct wl_client *client, uint32_t id,
    struct zen_shell *shell, struct zen_virtual_object *virtual_object)
{
  struct zen_cuboid_window *cuboid_window;
  struct wl_resource *resource;
  vec3 zero = GLM_VEC3_ZERO_INIT;
  mat4 identity = GLM_MAT4_IDENTITY_INIT;

  cuboid_window = zalloc(sizeof *cuboid_window);
  if (cuboid_window == NULL) {
    wl_client_post_no_memory(client);
    zen_log("cuboid window: failed to allocate memory\n");
    goto err;
  }

  resource = wl_resource_create(client, &zgn_cuboid_window_interface, 1, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("(cuboid window: failed to create a resource\n");
    goto err_resource;
  }

  wl_resource_set_implementation(resource, &cuboid_window_interface,
      cuboid_window, zen_cuboid_window_handle_destroy);

  cuboid_window->shell = shell;
  cuboid_window->resource = resource;
  cuboid_window->virtual_object = virtual_object;
  glm_vec3_copy(zero, cuboid_window->half_size);
  glm_mat4_copy(identity, cuboid_window->model_matrix);

  return cuboid_window;

err_resource:
  free(cuboid_window);

err:
  return NULL;
}

static void
zen_cuboid_window_destroy(struct zen_cuboid_window *cuboid_window)
{
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
