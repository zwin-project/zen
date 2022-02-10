#include <libzen-compositor/libzen-compositor.h>
#include <string.h>
#include <wayland-server.h>
#include <zen-shell/zen-shell.h>
#include <zigen-shell-server-protocol.h>

#include "background.h"
#include "cuboid-window.h"
#include "desktop.h"
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
    struct wl_resource* virtual_object_resource,
    struct wl_array* half_size_array, struct wl_array* quaternion_array)
{
  vec3 half_size;
  versor quaternion;
  struct zen_virtual_object* virtual_object;
  struct zen_cuboid_window* cuboid_window;

  virtual_object = wl_resource_get_user_data(virtual_object_resource);

  if (glm_vec3_from_wl_array(half_size, half_size_array) != 0) {
    wl_resource_post_error(resource, ZGN_SHELL_ERROR_INVALID_CUBOID_WINDOW,
        "half_size is not vec3");
    return;
  }

  if (glm_versor_from_wl_array(quaternion, quaternion_array) != 0) {
    wl_resource_post_error(resource, ZGN_SHELL_ERROR_INVALID_CUBOID_WINDOW,
        "quaternion is not vec4");
    return;
  }

  if (half_size[0] <= 0 || half_size[1] <= 0 || half_size[2] <= 0) {
    wl_resource_post_error(resource, ZGN_SHELL_ERROR_INVALID_CUBOID_WINDOW,
        "given cuboid window size has no volume");
    return;
  }

  cuboid_window =
      zen_cuboid_window_create(client, id, resource, virtual_object);
  if (cuboid_window == NULL) {
    zen_log("shell: failed to create a cuboid window\n");
    return;
  }

  vec3 initial_position = {
      0, 1.5, -(half_size[0] + half_size[1] + half_size[2])};
  glm_translate(cuboid_window->virtual_object->model_matrix, initial_position);

  zen_cuboid_window_configure(cuboid_window, half_size, quaternion);
}

static void
zen_shell_protocol_get_background(struct wl_client* client,
    struct wl_resource* resource, uint32_t id,
    struct wl_resource* virtual_object_resource)
{
  struct zen_virtual_object* virtual_object;
  struct zen_background* background;

  virtual_object = wl_resource_get_user_data(virtual_object_resource);

  background = zen_background_create(client, id, resource, virtual_object);
  if (background == NULL) {
    zen_log("shell: failed to create a background\n");
    return;
  }
}

static const struct zgn_shell_interface shell_interface = {
    .destroy = zen_shell_protocol_destroy,
    .get_cuboid_window = zen_shell_protocol_get_cuboid_window,
    .get_background = zen_shell_protocol_get_background,
};

static struct zen_virtual_object*
pick_virtual_object(struct zen_shell_base* shell_base, struct zen_ray* ray,
    vec3 local_ray_origin, vec3 local_ray_direction, float* distance)
{
  struct zen_shell* shell;
  struct zen_cuboid_window *cuboid_window, *focus_cuboid_window = NULL;
  float min_distance = FLT_MAX;
  vec3 ray_direction;

  shell = wl_container_of(shell_base, shell, base);
  zen_ray_get_direction(ray, ray_direction);

  wl_list_for_each(cuboid_window, &shell->cuboid_window_list, link)
  {
    mat4 transform;
    glm_quat_mat4(cuboid_window->quaternion, transform);
    glm_mat4_mul(
        cuboid_window->virtual_object->model_matrix, transform, transform);
    float d = zen_shell_ray_obb_intersection(
        ray->origin, ray_direction, cuboid_window->half_size, transform);

    if (d >= 0 && d < min_distance) {
      min_distance = d;
      focus_cuboid_window = cuboid_window;
    }
  }

  if (focus_cuboid_window) {
    mat4 model_matrix_inverse;
    vec3 rat_target, local_ray_target;
    glm_mat4_inv(focus_cuboid_window->virtual_object->model_matrix,
        model_matrix_inverse);
    glm_vec3_add(ray->origin, ray_direction, rat_target);
    glm_mat4_mulv3(model_matrix_inverse, rat_target, 1, local_ray_target);
    glm_mat4_mulv3(model_matrix_inverse, ray->origin, 1, local_ray_origin);
    glm_vec3_sub(local_ray_target, local_ray_origin, local_ray_direction);
    glm_vec3_normalize(local_ray_direction);
    *distance = min_distance;
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

WL_EXPORT void
zen_shell_unset_background(struct zen_shell* shell)
{
  struct zen_background *background, *tmp;

  wl_list_for_each_safe(background, tmp, &shell->background_list, link)
  {
    free(background->virtual_object->role);
    background->virtual_object->role = strdup("");
    zen_background_destroy(background);
  }
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
  wl_list_init(&shell->background_list);
  shell->global = global;
  shell->interface = &zen_desktop_shell_interface;

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
