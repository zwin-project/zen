#include "board-move.h"

#include <cglm/vec2.h>
#include <zen-common.h>

#include "board.h"
#include "zen/appearance/board.h"
#include "zen/appearance/ray.h"
#include "zen/server.h"

static void zns_board_move_ray_grab_destroy(
    struct zns_board_move_ray_grab *self);

static void
zns_board_move_ray_grab_motion_relative(struct zn_ray_grab *grab_base,
    vec3 origin, float polar, float azimuthal, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zns_board_move_ray_grab *self = zn_container_of(grab_base, self, base);

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

  vec3 tip, next_center;
  struct zn_board *zn_board = self->zns_board->zn_board;
  zn_ray_get_tip(self->base.ray, tip);
  glm_vec3_sub(tip, self->tip, next_center);

  zn_board_move(zn_board, next_center, zn_board->geometry.size,
      zn_board->geometry.quaternion);

  zna_board_commit(zn_board->appearance);
}

static void
zns_board_move_ray_grab_button(struct zn_ray_grab *grab_base,
    uint32_t time_msec, uint32_t button, enum zgn_ray_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);
  struct zns_board_move_ray_grab *self = zn_container_of(grab_base, self, base);

  if (state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    zn_ray_end_grab(self->base.ray);
  }
}

static void
zns_board_move_ray_grab_rebase(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
}

static void
zns_board_move_ray_grab_cancel(struct zn_ray_grab *grab_base)
{
  struct zns_board_move_ray_grab *self = zn_container_of(grab_base, self, base);
  zns_board_move_ray_grab_destroy(self);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_board_move_ray_grab_motion_relative,
    .button = zns_board_move_ray_grab_button,
    .rebase = zns_board_move_ray_grab_rebase,
    .cancel = zns_board_move_ray_grab_cancel,
};

static void
zns_board_move_ray_grab_handle_zns_board_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_board_move_ray_grab *self =
      zn_container_of(listener, self, zns_board_destroy_listener);

  if (self->base.ray->grab->interface == &implementation) {
    zn_ray_end_grab(self->base.ray);
  }
}

struct zns_board_move_ray_grab *
zns_board_move_ray_grab_create(struct zns_board *zns_board)
{
  struct zns_board_move_ray_grab *self;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_ray *ray = server->scene->ray;
  float u, v;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  if (zns_board_ray_cast(zns_board, ray->origin, ray->direction, &u, &v) ==
      FLT_MAX) {
    goto err_free;
  }

  self->base.interface = &implementation;
  self->zns_board = zns_board;
  self->tip[0] = zns_board->zn_board->geometry.size[0] * (u - 0.5f);
  self->tip[1] = zns_board->zn_board->geometry.size[1] * (v - 0.5f);
  self->tip[2] = 0;

  self->zns_board_destroy_listener.notify =
      zns_board_move_ray_grab_handle_zns_board_destroy;
  wl_signal_add(&zns_board->events.destroy, &self->zns_board_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zns_board_move_ray_grab_destroy(struct zns_board_move_ray_grab *self)
{
  wl_list_remove(&self->zns_board_destroy_listener.link);
  free(self);
}
