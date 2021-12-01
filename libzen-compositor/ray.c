#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>

WL_EXPORT struct zen_ray*
zen_ray_create(struct zen_seat* seat)
{
  struct zen_ray* ray;
  struct zen_render_item* render_item;
  vec3 initial_origin = {0.3f, 1.1f, -0.3f};

  ray = zalloc(sizeof *ray);
  if (ray == NULL) {
    zen_log("ray: failed to allocate memory\n");
    goto err;
  }

  render_item = zen_ray_render_item_create(seat->compositor->renderer, ray);
  if (render_item == NULL) {
    zen_log("ray: failed create a render item\n");
    goto err_render_item;
  }

  wl_list_init(&ray->ray_client_list);
  wl_signal_init(&ray->destroy_signal);
  glm_vec3_copy(initial_origin, ray->origin);
  ray->angle.polar = GLM_PI / 3;
  ray->angle.azimuthal = GLM_PI * 1.3;
  ray->render_item = render_item;

  return ray;

err_render_item:
  free(ray);

err:
  return NULL;
}

WL_EXPORT void
zen_ray_destroy(struct zen_ray* ray)
{
  zen_ray_render_item_destroy(ray->render_item);
  wl_signal_emit(&ray->destroy_signal, NULL);
  free(ray);
}
