#include "move.h"

#include <zen-common.h>

#include "zen/appearance/ray.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

static void zns_move_ray_grab_destroy(struct zns_move_ray_grab *self);

static void
zns_move_ray_grab_motion_relative(struct zn_ray_grab *grab_base, vec3 origin,
    float polar, float azimuthal, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zns_move_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_virtual_object *zn_virtual_object =
      self->bounded->zgnr_bounded->virtual_object->user_data;

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

  zna_ray_commit(self->base.ray->appearance);

  vec3 tip, next_virtual_object_position;
  zn_ray_get_tip(self->base.ray, tip);
  glm_vec3_sub(tip, self->local_tip, next_virtual_object_position);

  zn_virtual_object_move(zn_virtual_object, next_virtual_object_position,
      zn_virtual_object->quaternion);
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

  if (self->base.ray->grab->interface == &implementation) {
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

  self->base.interface = &implementation;
  self->bounded = bounded;

  self->bounded_destroy_listener.notify =
      zns_move_ray_grab_handle_bounded_destroy;
  wl_signal_add(&bounded->events.destroy, &self->bounded_destroy_listener);

  vec3 tip;
  zn_ray_get_tip(server->input_manager->seat->ray, tip);
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
