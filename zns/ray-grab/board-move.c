#include "board-move.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/vec2.h>
#include <zen-common.h>

#include "board.h"
#include "seat-capsule.h"
#include "shell.h"
#include "zen/appearance/board.h"
#include "zen/appearance/cursor.h"
#include "zen/appearance/ray.h"
#include "zen/cursor.h"
#include "zen/server.h"
#include "zen/view.h"

static void zns_board_move_ray_grab_destroy(
    struct zns_board_move_ray_grab *self);

static void
zns_board_move_ray_grab_motion_relative(struct zn_ray_grab *grab_base,
    vec3 origin, float polar, float azimuthal, uint32_t time_msec)
{
  UNUSED(time_msec);
  struct zns_board_move_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_board *zn_board = self->zns_board->zn_board;
  struct zn_server *server = zn_server_get_singleton();
  struct zns_seat_capsule *seat_capsule = server->shell->seat_capsule;

  if (!zn_assert(origin[0] == 0 && origin[1] == 0 && origin[2] == 0,
          "ZEN is not supporting 6DoF devices yet")) {
    return;
  }

  float next_board_polar = self->zns_board->seat_capsule_polar + polar;
  if (next_board_polar < 0)
    next_board_polar = 0;
  else if (next_board_polar > M_PI)
    next_board_polar = M_PI;

  float next_board_azimuthal =
      self->zns_board->seat_capsule_azimuthal + azimuthal;
  while (next_board_azimuthal >= 2 * M_PI) next_board_azimuthal -= 2 * M_PI;
  while (next_board_azimuthal < 0) next_board_azimuthal += 2 * M_PI;

  zns_seat_capsule_move_board(
      seat_capsule, self->zns_board, next_board_azimuthal, next_board_polar);

  zna_board_commit(zn_board->appearance);

  {  // move ray
    vec3 next_tip, next_origin, next_direction;
    float next_polar, next_azimuthal, next_length;
    glm_vec3_add(self->base.ray->origin, origin, next_origin);
    glm_mat4_mulv3(zn_board->geometry.transform, self->local_tip, 1, next_tip);
    glm_vec3_sub(next_tip, self->base.ray->origin, next_direction);
    next_length = glm_vec3_distance(GLM_VEC3_ZERO, next_direction);
    glm_vec3_normalize(next_direction);
    next_azimuthal = atan2f(-next_direction[2], next_direction[0]);
    float r = sqrtf(powf(next_direction[2], 2) + powf(next_direction[0], 2));
    next_polar = atan2(r, next_direction[1]);

    zn_ray_move(self->base.ray, next_origin, next_polar, next_azimuthal);

    zn_ray_set_length(self->base.ray, next_length);

    zna_ray_commit(self->base.ray->appearance);
  }

  {  // move view
    struct zn_view *view;
    wl_list_for_each (view, &zn_board->view_list, board_link) {
      zn_view_move(view, zn_board, view->x, view->y);
      zn_view_commit_appearance(view);
    }
  }

  {  // move cursor
    struct zn_cursor *cursor = server->scene->cursor;
    zn_cursor_move(cursor, zn_board, cursor->x, cursor->y);

    zn_cursor_commit_appearance(cursor);
  }
}

static void
zns_board_move_ray_grab_button(struct zn_ray_grab *grab_base,
    uint32_t time_msec, uint32_t button, enum wlr_button_state state)
{
  UNUSED(time_msec);
  UNUSED(button);
  struct zns_board_move_ray_grab *self = zn_container_of(grab_base, self, base);

  if (state == WLR_BUTTON_RELEASED) {
    zn_ray_end_grab(self->base.ray);
  }
}

static void
zns_board_move_ray_grab_axis(struct zn_ray_grab *grab_base, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(grab_base);
  UNUSED(time_msec);
  UNUSED(source);
  UNUSED(orientation);
  UNUSED(delta);
  UNUSED(delta_discrete);
}

static void
zns_board_move_ray_grab_frame(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
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
    .axis = zns_board_move_ray_grab_axis,
    .frame = zns_board_move_ray_grab_frame,
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

  if (self->base.ray->grab->impl == &implementation) {
    zn_ray_end_grab(self->base.ray);
  }
}

static struct zns_board_move_ray_grab *
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

  self->base.impl = &implementation;
  self->zns_board = zns_board;
  self->local_tip[0] = zns_board->zn_board->geometry.size[0] * (u - 0.5f);
  self->local_tip[1] = zns_board->zn_board->geometry.size[1] * v;
  self->local_tip[2] = 0;

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

void
zns_board_move_ray_grab_start(struct zn_ray *ray, struct zns_board *zns_board)
{
  struct zns_board_move_ray_grab *self =
      zns_board_move_ray_grab_create(zns_board);
  if (!self) return;

  zn_ray_start_grab(ray, &self->base);
}
