#include <cglm/cglm.h>
#include <libzen-compositor/libzen-compositor.h>
#include <wayland-server.h>
#include <zigen-server-protocol.h>

#include "ray-client.h"
#include "virtual-object.h"

static void
zen_ray_enter(struct zen_ray* ray, struct zen_virtual_object* virtual_object)
{
  struct zen_ray_client* ray_client;
  struct wl_resource* resource;
  struct wl_array origin_array, direction_array;
  uint32_t serial;

  ray_client = zen_ray_client_find(
      wl_resource_get_client(virtual_object->resource), ray);
  if (ray_client == NULL) return;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  serial = wl_display_next_serial(virtual_object->compositor->display);
  wl_resource_for_each(resource, &ray_client->resource_list)
  {
    zgn_ray_send_enter(resource, serial, virtual_object->resource,
        &origin_array, &direction_array);
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

static void
zen_ray_motion(struct zen_ray* ray, const struct timespec* time)
{
  struct zen_virtual_object* virtual_object;
  struct zen_ray_client* ray_client;
  struct wl_resource* resource;
  struct wl_array origin_array, direction_array;
  uint32_t msecs;

  virtual_object = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);
  if (virtual_object == NULL) return;

  ray_client = zen_ray_client_find(
      wl_resource_get_client(virtual_object->resource), ray);
  if (ray_client == NULL) return;

  wl_array_init(&origin_array);
  wl_array_init(&direction_array);

  glm_vec3_to_wl_array(ray->local_origin, &origin_array);
  glm_vec3_to_wl_array(ray->local_direction, &direction_array);

  msecs = timespec_to_msec(time);

  wl_resource_for_each(resource, &ray_client->resource_list)
  {
    zgn_ray_send_motion(resource, msecs, &origin_array, &direction_array);
  }

  wl_array_release(&origin_array);
  wl_array_release(&direction_array);
}

static void
zen_ray_leave(struct zen_ray* ray)
{
  struct zen_virtual_object* virtual_object;
  struct zen_ray_client* ray_client;
  struct wl_resource* resource;
  uint32_t serial;

  virtual_object = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);
  if (virtual_object == NULL) return;

  ray_client = zen_ray_client_find(
      wl_resource_get_client(virtual_object->resource), ray);
  if (ray_client == NULL) return;

  serial = wl_display_next_serial(virtual_object->compositor->display);

  wl_resource_for_each(resource, &ray_client->resource_list)
  {
    zgn_ray_send_leave(resource, serial, virtual_object->resource);
  }
}

static void
default_grab_focus(struct zen_ray_grab* grab)
{
  struct zen_ray* ray = grab->ray;
  struct zen_shell_base* shell_base = ray->seat->compositor->shell_base;
  struct zen_virtual_object *virtual_object, *current_focus;

  current_focus = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);

  virtual_object = shell_base->pick_virtual_object(shell_base, ray,
      ray->local_origin, ray->local_direction, &ray->target_distance);

  if (current_focus == virtual_object) return;

  if (current_focus) zen_ray_leave(ray);
  if (virtual_object) zen_ray_enter(ray, virtual_object);

  if (virtual_object)
    zen_weak_link_set(
        &ray->focus_virtual_object_link, virtual_object->resource);
  else
    zen_weak_link_unset(&ray->focus_virtual_object_link);

  if (current_focus) zen_virtual_object_render_commit(current_focus);
  if (virtual_object) zen_virtual_object_render_commit(virtual_object);
}

static void
default_grab_motion(struct zen_ray_grab* grab, const struct timespec* time,
    struct zen_ray_motion_event* event)
{
  struct zen_ray* ray = grab->ray;
  zen_ray_move(ray, event);

  zen_ray_motion(ray, time);
}

static void
default_grab_button(struct zen_ray_grab* grab, const struct timespec* time,
    uint32_t button, uint32_t state)
{
  struct zen_ray* ray = grab->ray;
  struct zen_virtual_object* focus_virtual_object;
  struct zen_ray_client* ray_client;
  struct wl_resource* resource;
  uint32_t serial, msec;

  serial = wl_display_next_serial(ray->seat->compositor->display);

  focus_virtual_object =
      zen_weak_link_get_user_data(&ray->focus_virtual_object_link);
  if (focus_virtual_object == NULL) return;

  ray_client = zen_ray_client_find(
      wl_resource_get_client(focus_virtual_object->resource), ray);
  if (ray_client == NULL) return;

  msec = timespec_to_msec(time);

  wl_resource_for_each(resource, &ray_client->resource_list)
  {
    zgn_ray_send_button(resource, serial, msec, button, state);
    wl_client_flush(wl_resource_get_client(resource));
  }
}

static void
default_grab_cancel(struct zen_ray_grab* grab)
{
  UNUSED(grab);
}

static const struct zen_ray_grab_interface default_ray_grab_interface = {
    .focus = default_grab_focus,
    .motion = default_grab_motion,
    .button = default_grab_button,
    .cancel = default_grab_cancel,
};

WL_EXPORT void
zen_ray_grab_start(struct zen_ray* ray, struct zen_ray_grab* grab)
{
  struct zen_virtual_object* current_focus;
  if (ray->grab != &ray->default_grab) return;

  ray->grab = grab;
  ray->grab->ray = ray;
  ray->grab->interface->focus(ray->grab);

  current_focus = zen_weak_link_get_user_data(&ray->focus_virtual_object_link);

  zen_ray_leave(ray);
  zen_weak_link_unset(&ray->focus_virtual_object_link);
  if (current_focus) zen_virtual_object_render_commit(current_focus);
}

WL_EXPORT void
zen_ray_grab_end(struct zen_ray* ray)
{
  ray->grab = &ray->default_grab;
}

WL_EXPORT void
zen_ray_get_direction(struct zen_ray* ray, vec3 direction)
{
  double sin_theta = sin(ray->angle.polar);
  direction[0] = sin_theta * cos(ray->angle.azimuthal);
  direction[1] = cos(ray->angle.polar);
  direction[2] = sin_theta * sin(ray->angle.azimuthal);
}

WL_EXPORT void
zen_ray_move(struct zen_ray* ray, struct zen_ray_motion_event* event)
{
  glm_vec3_add(ray->origin, event->delta_origin, ray->origin);

  ray->angle.polar += event->delta_polar_angle;
  while (ray->angle.polar >= GLM_2PI) ray->angle.polar -= GLM_2PI;
  while (ray->angle.polar < 0) ray->angle.polar += GLM_2PI;

  ray->angle.azimuthal += event->delta_azimuthal_angle;
  while (ray->angle.azimuthal >= GLM_2PI) ray->angle.azimuthal -= GLM_2PI;
  while (ray->angle.azimuthal < 0) ray->angle.azimuthal += GLM_2PI;

  ray->render_item->commit(ray->render_item);
  ray->grab->interface->focus(ray->grab);
}

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

  ray->seat = seat;
  ray->grab = &ray->default_grab;
  ray->default_grab.interface = &default_ray_grab_interface;
  ray->default_grab.ray = ray;
  ray->button_count = 0;
  zen_weak_link_init(&ray->focus_virtual_object_link);
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
  ray->grab->interface->cancel(ray->grab);
  zen_weak_link_unset(&ray->focus_virtual_object_link);
  zen_ray_render_item_destroy(ray->render_item);
  wl_signal_emit(&ray->destroy_signal, NULL);
  free(ray);
}
