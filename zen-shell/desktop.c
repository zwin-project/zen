#include "desktop.h"

#include <libzen-compositor/libzen-compositor.h>
#include <zigen-server-protocol.h>

#include "cuboid-window.h"

struct zen_desktop_move_grab {
  struct zen_ray_grab base;
  struct zen_weak_link cuboid_window_link;
};

static void zen_desktop_move_grab_destroy(
    struct zen_desktop_move_grab *move_grab);

static void
move_grab_ray_focus(struct zen_ray_grab *grab)
{
  UNUSED(grab);
}

static void
move_grab_ray_motion(struct zen_ray_grab *grab, const struct timespec *time,
    struct zen_ray_motion_event *event)
{
  UNUSED(time);
  UNUSED(event);
  struct zen_ray *ray = grab->ray;
  struct zen_desktop_move_grab *move_grab =
      wl_container_of(grab, move_grab, base);
  struct zen_cuboid_window *cuboid_window =
      zen_weak_link_get_user_data(&move_grab->cuboid_window_link);
  vec3 old_ray_tip, new_ray_tip, ray_direction, delta;

  zen_ray_get_direction(ray, ray_direction);
  glm_vec3_scale(ray_direction, ray->target_distance, old_ray_tip);
  glm_vec3_add(ray->origin, old_ray_tip, old_ray_tip);

  zen_ray_move(ray, event);

  if (cuboid_window == NULL) return;

  zen_ray_get_direction(ray, ray_direction);
  glm_vec3_scale(ray_direction, ray->target_distance, new_ray_tip);
  glm_vec3_add(ray->origin, new_ray_tip, new_ray_tip);

  glm_vec3_sub(new_ray_tip, old_ray_tip, delta);

  glm_translate(cuboid_window->virtual_object->model_matrix, delta);
}

static void
move_grab_ray_button(struct zen_ray_grab *grab, const struct timespec *time,
    uint32_t button, enum zgn_ray_button_state state)
{
  UNUSED(time);
  UNUSED(button);

  struct zen_desktop_move_grab *move_grab =
      wl_container_of(grab, move_grab, base);

  if (grab->ray->button_count == 0 && state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    zen_ray_grab_end(grab->ray);
    zen_desktop_move_grab_destroy(move_grab);
  }
}

static void
move_grab_ray_cancel(struct zen_ray_grab *grab)
{
  struct zen_desktop_move_grab *move_grab =
      wl_container_of(grab, move_grab, base);

  zen_ray_grab_end(grab->ray);
  zen_desktop_move_grab_destroy(move_grab);
}

static const struct zen_ray_grab_interface move_grab_interface = {
    .focus = move_grab_ray_focus,
    .motion = move_grab_ray_motion,
    .button = move_grab_ray_button,
    .cancel = move_grab_ray_cancel,
};

static struct zen_desktop_move_grab *
zen_desktop_move_grab_create(struct zen_cuboid_window *cuboid_window)
{
  struct zen_desktop_move_grab *move_grab;

  move_grab = zalloc(sizeof *move_grab);
  move_grab->base.interface = &move_grab_interface;
  zen_weak_link_init(&move_grab->cuboid_window_link);
  zen_weak_link_set(&move_grab->cuboid_window_link, cuboid_window->resource);

  return move_grab;
}

static void
zen_desktop_move_grab_destroy(struct zen_desktop_move_grab *move_grab)
{
  zen_weak_link_unset(&move_grab->cuboid_window_link);
  free(move_grab);
}

static void
desktop_cuboid_window_move(struct zen_cuboid_window *cuboid_window,
    struct zen_seat *seat, uint32_t serial)
{
  UNUSED(cuboid_window);
  UNUSED(serial);
  struct zen_ray *ray = seat->ray;
  struct zen_desktop_move_grab *move_grab;

  if (ray == NULL) return;

  move_grab = zen_desktop_move_grab_create(cuboid_window);

  if (ray->button_count > 0 && ray->grab_serial == serial &&
      ray->focus_virtual_object_link.resource ==
          cuboid_window->virtual_object->resource) {
    zen_ray_grab_start(ray, &move_grab->base);
  }
}

static void
desktop_cuboid_window_rotate(
    struct zen_cuboid_window *cuboid_window, versor quaternion)
{
  struct zen_ray *ray = cuboid_window->shell->compositor->seat->ray;
  zen_cuboid_window_configure(
      cuboid_window, cuboid_window->half_size, quaternion);

  if (ray == NULL) return;

  ray->grab->interface->focus(ray->grab);
}

WL_EXPORT struct zen_desktop_api zen_desktop_shell_interface = {
    .move = desktop_cuboid_window_move,
    .rotate = desktop_cuboid_window_rotate,
};
