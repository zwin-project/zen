#include "default.h"

#include <cglm/vec3.h>
#include <zen-common.h>
#include <zigen-protocol.h>

#include "bounded.h"
#include "shell.h"
#include "zen/scene/virtual-object.h"
#include "zen/server.h"

/**
 * @param bounded is nullable
 */
static void
zns_default_ray_grab_focus(
    struct zns_default_ray_grab* self, struct zns_bounded* bounded)
{
  if (self->focus == bounded) return;

  struct zn_server* server = zn_server_get_singleton();
  struct zgnr_seat* seat = server->input_manager->seat->zgnr_seat;

  if (self->focus) {
    wl_list_remove(&self->focus_destroy_listener.link);
    wl_list_init(&self->focus_destroy_listener.link);
    zgnr_seat_send_ray_leave(seat, self->focus->zgnr_bounded->virtual_object);
  }

  if (bounded) {
    wl_signal_add(&bounded->events.destroy, &self->focus_destroy_listener);
    struct zn_virtual_object* zn_virtual_object =
        bounded->zgnr_bounded->virtual_object->user_data;

    vec3 ray_tip, local_origin, local_tip, local_direction;
    mat4 invert;
    glm_mat4_inv(zn_virtual_object->model_matrix, invert);
    zn_ray_get_tip(self->base.ray, 1, ray_tip);
    glm_mat4_mulv3(invert, ray_tip, 1, local_tip);
    glm_mat4_mulv3(invert, self->base.ray->origin, 1, local_origin);
    glm_vec3_sub(local_tip, local_origin, local_direction);
    zgnr_seat_send_ray_enter(seat, bounded->zgnr_bounded->virtual_object,
        local_origin, local_direction);
  }

  self->focus = bounded;
}

static void
zns_default_ray_grab_send_motion(struct zns_default_ray_grab* self)
{
  // TODO: motion
  UNUSED(self);
}

static void
zns_default_ray_grab_motion_relative(
    struct zn_ray_grab* grab_base, vec3 origin, float polar, float azimuthal)
{
  struct zns_default_ray_grab* self = zn_container_of(grab_base, self, base);
  struct zns_bounded* bounded;

  float next_polar = self->base.ray->angle.polar + polar;
  if (next_polar < 0)
    next_polar = 0;
  else if (next_polar > M_PI)
    next_polar = M_PI;

  float next_azimuthal = self->base.ray->angle.azimuthal + azimuthal;
  while (next_azimuthal >= 2 * M_PI) next_azimuthal -= 2 * M_PI;
  while (next_azimuthal < 0) next_azimuthal += 2 * M_PI;

  vec3 next_origin;
  glm_vec3_add(self->base.ray->origin, origin, next_origin);

  zn_ray_move(self->base.ray, next_origin, next_polar, next_azimuthal);

  float distance;
  bounded = zn_shell_ray_cast(self->shell, self->base.ray, &distance);

  zns_default_ray_grab_focus(self, bounded);

  zns_default_ray_grab_send_motion(self);
}

static void
zns_default_ray_grab_cancel(struct zn_ray_grab* grab)
{
  UNUSED(grab);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_default_ray_grab_motion_relative,
    .cancel = zns_default_ray_grab_cancel,
};

void
zns_default_ray_grab_handle_focus_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);
  struct zns_default_ray_grab* self =
      zn_container_of(listener, self, focus_destroy_listener);

  zns_default_ray_grab_focus(self, NULL);
}

void
zns_default_ray_grab_init(
    struct zns_default_ray_grab* self, struct zn_shell* shell)
{
  self->base.interface = &implementation;
  self->shell = shell;
  self->focus = NULL;

  self->focus_destroy_listener.notify =
      zns_default_ray_grab_handle_focus_destroy;
  wl_list_init(&self->focus_destroy_listener.link);
}

void
zns_default_ray_grab_fini(struct zns_default_ray_grab* self)
{
  wl_list_remove(&self->focus_destroy_listener.link);
}
