#include "move.h"

#include <zen-common.h>

#include "seat-capsule.h"
#include "shell.h"
#include "zen/appearance/ray.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

static void zns_move_ray_grab_destroy(struct zns_move_ray_grab *self);

static void
zns_move_ray_grab_motion_relative(struct zn_ray_grab *grab_base, vec3 origin,
    float polar, float azimuthal, uint32_t time_msec)
{
  UNUSED(time_msec);
  if (!zn_assert(origin[0] == 0 && origin[1] == 0 && origin[2] == 0,
          "ZEN is not supporting 6DoF devices yet")) {
    return;
  }

  struct zn_server *server = zn_server_get_singleton();
  struct zns_seat_capsule *seat_capsule = server->shell->seat_capsule;
  struct zns_move_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_virtual_object *virtual_object =
      self->bounded->zgnr_bounded->virtual_object->user_data;

  float next_bounded_polar = self->bounded->seat_capsule_polar + polar;
  if (next_bounded_polar < 0)
    next_bounded_polar = 0;
  else if (next_bounded_polar > M_PI)
    next_bounded_polar = M_PI;

  float next_bounded_azimuthal =
      self->bounded->seat_capsule_azimuthal + azimuthal;
  while (next_bounded_azimuthal >= 2 * M_PI) next_bounded_azimuthal -= 2 * M_PI;
  while (next_bounded_azimuthal < 0) next_bounded_azimuthal += 2 * M_PI;

  zns_seat_capsule_move_bounded(
      seat_capsule, self->bounded, next_bounded_azimuthal, next_bounded_polar);

  vec3 next_tip, next_origin, next_direction;
  float next_polar, next_azimuthal, next_length;
  glm_vec3_add(self->base.ray->origin, origin, next_origin);
  glm_mat4_mulv3(virtual_object->model_matrix, self->local_tip, 1, next_tip);
  glm_vec3_sub(next_tip, next_origin, next_direction);
  next_length = glm_vec3_distance(GLM_VEC3_ZERO, next_direction);
  glm_vec3_normalize(next_direction);
  next_azimuthal = atan2f(-next_direction[2], next_direction[0]);
  float r = sqrtf(powf(next_direction[2], 2) + powf(next_direction[0], 2));
  next_polar = atan2(r, next_direction[1]);

  zn_ray_move(self->base.ray, next_origin, next_polar, next_azimuthal);

  zn_ray_set_length(self->base.ray, next_length);

  zna_ray_commit(self->base.ray->appearance);
}

static void
zns_move_ray_grab_button(struct zn_ray_grab *grab_base, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state)
{
  struct zns_move_ray_grab *self = zn_container_of(grab_base, self, base);
  UNUSED(time_msec);
  UNUSED(button);

  if (state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    zn_ray_end_grab(self->base.ray);
  }
}

static void
zns_move_ray_grab_rebase(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
}

static void
zns_move_ray_grab_cancel(struct zn_ray_grab *grab_base)
{
  struct zns_move_ray_grab *self = zn_container_of(grab_base, self, base);
  zns_move_ray_grab_destroy(self);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_move_ray_grab_motion_relative,
    .button = zns_move_ray_grab_button,
    .rebase = zns_move_ray_grab_rebase,
    .cancel = zns_move_ray_grab_cancel,
};

static void
zns_move_ray_grab_handle_bounded_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_move_ray_grab *self =
      zn_container_of(listener, self, bounded_destroy_listener);

  if (self->base.ray->grab->impl == &implementation) {
    zn_ray_end_grab(self->base.ray);
  }
}

struct zns_move_ray_grab *
zns_move_ray_grab_create(struct zns_bounded *bounded)
{
  struct zns_move_ray_grab *self;
  struct zn_virtual_object *zn_virtual_object =
      bounded->zgnr_bounded->virtual_object->user_data;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->base.impl = &implementation;
  self->bounded = bounded;

  self->bounded_destroy_listener.notify =
      zns_move_ray_grab_handle_bounded_destroy;
  wl_signal_add(&bounded->events.destroy, &self->bounded_destroy_listener);

  vec3 tip;
  zn_ray_get_tip(server->scene->ray, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, self->local_tip);

  return self;

err:
  return NULL;
}

static void
zns_move_ray_grab_destroy(struct zns_move_ray_grab *self)
{
  wl_list_remove(&self->bounded_destroy_listener.link);
  free(self);
}
