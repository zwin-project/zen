#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>
#include <zigen-shell-server-protocol.h>

#include "cuboid-window.h"
#include "intersection.h"

char* zen_shell_type = "zen_shell";

static void
zen_shell_protocol_destroy(
    struct wl_client* client, struct wl_resource* resource)
{
  UNUSED(client);
  wl_resource_destroy(resource);
}

static void
zen_shell_protocol_get_cuboid_window(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object_resource, struct wl_array* half_size)
{
  vec3 half_size_vec;
  struct zen_virtual_object* virtual_object;
  struct zen_cuboid_window* cuboid_window;

  virtual_object = wl_resource_get_user_data(virtual_object_resource);

  if (glm_vec3_from_wl_array(half_size_vec, half_size) != 0) {
    wl_resource_post_error(resource, ZGN_SHELL_ERROR_INVALID_CUBOID_WINDOW_SIZE,
        "half_size is not vec3");
    return;
  }

  if (half_size_vec[0] <= 0 || half_size_vec[1] <= 0 || half_size_vec[2] <= 0) {
    wl_resource_post_error(resource, ZGN_SHELL_ERROR_INVALID_CUBOID_WINDOW_SIZE,
        "given cuboid window size has no volume");
    return;
  }

  cuboid_window =
      zen_cuboid_window_create(client, id, resource, virtual_object);
  if (cuboid_window == NULL) {
    zen_log("shell: failed to create a cuboid window\n");
    return;
  }

  zen_cuboid_window_configure(cuboid_window, half_size_vec);
}

static const struct zgn_shell_interface shell_interface = {
    .destroy = zen_shell_protocol_destroy,
    .get_cuboid_window = zen_shell_protocol_get_cuboid_window,
};

static struct zen_virtual_object*
pick_virtual_object(struct zen_shell_base* shell_base, struct zen_ray* ray)
{
  struct zen_shell* shell;
  struct zen_cuboid_window *cuboid_window, *focus_cuboid_window = NULL;
  float min_distance = FLT_MAX;
  vec3 ray_direction;

  shell = wl_container_of(shell_base, shell, base);
  zen_ray_get_direction(ray, ray_direction);

  wl_list_for_each(cuboid_window, &shell->cuboid_window_list, link)
  {
    float d = zen_shell_ray_obb_intersection(ray->origin, ray_direction,
        cuboid_window->half_size, cuboid_window->virtual_object->model_matrix);

    if (d >= 0 && d < min_distance) {
      min_distance = d;
      focus_cuboid_window = cuboid_window;
    }
  }

  return focus_cuboid_window ? focus_cuboid_window->virtual_object : NULL;
}

static void
zen_shell_bind(
    struct wl_client* client, void* data, uint32_t version, uint32_t id)
{
  struct zen_shell* shell = data;
  struct wl_resource* resource;

  resource = wl_resource_create(client, &zgn_shell_interface, version, id);
  if (resource == NULL) {
    wl_client_post_no_memory(client);
    zen_log("shell: failed to create a resource\n");
    return;
  }

  wl_resource_set_implementation(resource, &shell_interface, shell, NULL);
}

WL_EXPORT struct zen_shell_base*
zen_shell_create(struct zen_compositor* compositor)
{
  struct zen_shell* shell;
  struct wl_global* global;

  shell = zalloc(sizeof *shell);
  if (shell == NULL) {
    zen_log("shell: failed to allocate memory\n");
    goto err;
  }

  global = wl_global_create(
      compositor->display, &zgn_shell_interface, 1, shell, zen_shell_bind);
  if (global == NULL) {
    zen_log("shell: failed to create a zen_shell global\n");
    goto err_global;
  }

  shell->base.type = zen_shell_type;
  shell->base.pick_virtual_object = pick_virtual_object;
  shell->compositor = compositor;
  wl_list_init(&shell->cuboid_window_list);
  shell->global = global;

  return &shell->base;

err_global:
  free(shell);

err:
  return NULL;
}

WL_EXPORT void
zen_shell_destroy(struct zen_shell_base* shell_base)
{
  assert(shell_base->type == zen_shell_type);
  struct zen_shell* shell = wl_container_of(shell_base, shell, base);

  free(shell);
}
