#include "board.h"

#include <cglm/quat.h>
#include <linux/input.h>
#include <zen-common.h>
#include <zgnr/intersection.h>

#include "ray-grab/board-move.h"
#include "shell.h"
#include "zen/cursor.h"
#include "zen/server.h"

static void zns_board_destroy(struct zns_board *self);

float
zns_board_ray_cast(
    struct zns_board *self, vec3 origin, vec3 direction, float *u, float *v)
{
  float x = self->zn_board->geometry.size[0] / 2.f;
  vec3 v0 = {-x, 0, 0};
  vec3 v1 = {+x, 0, 0};
  vec3 v2 = {-x, self->zn_board->geometry.size[1], 0};

  glm_mat4_mulv3(self->zn_board->geometry.transform, v0, 1, v0);
  glm_mat4_mulv3(self->zn_board->geometry.transform, v1, 1, v1);
  glm_mat4_mulv3(self->zn_board->geometry.transform, v2, 1, v2);

  return zgnr_intersection_ray_parallelogram(
      origin, direction, v0, v1, v2, u, v, false);

  // TODO: Take into account the overhanging views
}

static bool
zns_board_node_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_board *self = user_data;

  float u, v;
  float d = zns_board_ray_cast(self, origin, direction, &u, &v);

  if (d >= *distance) return false;

  *distance = d;

  return true;
}

static bool
zns_board_node_ray_motion(void *user_data, vec3 origin, vec3 direction,
    uint32_t time_msec, mat4 transform)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_board *self = user_data;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->scene->cursor;
  float u, v;
  double effective_width, effective_height;
  double effective_x, effective_y;

  (void)zns_board_ray_cast(self, origin, direction, &u, &v);

  zn_board_get_effective_size(
      self->zn_board, &effective_width, &effective_height);

  effective_x = effective_width * u;
  effective_y = effective_height * (1 - v);

  cursor->grab->impl->motion_absolute(
      cursor->grab, self->zn_board, effective_x, effective_y, time_msec);

  return true;
}

static bool
zns_board_node_ray_enter(
    void *user_data, vec3 origin, vec3 direction, mat4 transform)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_board *self = user_data;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->scene->cursor;
  float u, v;
  double effective_width, effective_height;
  double effective_x, effective_y;

  (void)zns_board_ray_cast(self, origin, direction, &u, &v);

  zn_board_get_effective_size(
      self->zn_board, &effective_width, &effective_height);

  effective_x = effective_width * u;
  effective_y = effective_height * (1 - v);

  cursor->grab->impl->enter(
      cursor->grab, self->zn_board, effective_x, effective_y);

  return true;
}

static bool
zns_board_node_ray_leave(void *user_data, mat4 transform)
{
  UNUSED(user_data);
  UNUSED(transform);  // must be identity matrix

  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->scene->cursor;

  cursor->grab->impl->leave(cursor->grab);

  return true;
}

static bool
zns_board_node_ray_button(void *user_data, uint32_t time_msec, uint32_t button,
    enum zgn_ray_button_state state, mat4 transform)
{
  UNUSED(transform);  // must be identity matrix

  struct zns_board *self = user_data;
  struct zn_server *server = zn_server_get_singleton();
  struct zn_cursor *cursor = server->scene->cursor;
  struct wlr_surface *surface;

  if (cursor->board == NULL) return true;

  surface = zn_board_get_surface_at(
      cursor->board, cursor->x, cursor->y, NULL, NULL, NULL);

  if (surface) {
    enum wlr_button_state wlr_state;
    if (state == ZGN_RAY_BUTTON_STATE_PRESSED) {
      wlr_state = WLR_BUTTON_PRESSED;
    } else {
      wlr_state = WLR_BUTTON_RELEASED;
    }

    cursor->grab->impl->button(cursor->grab, time_msec, button, wlr_state);
  } else if (state == ZGN_RAY_BUTTON_STATE_PRESSED && button == BTN_LEFT) {
    zns_board_move_ray_grab_start(server->scene->ray, self);
  }

  return true;
}

static const struct zns_node_interface node_implementation = {
    .ray_cast = zns_board_node_ray_cast,
    .ray_motion = zns_board_node_ray_motion,
    .ray_enter = zns_board_node_ray_enter,
    .ray_leave = zns_board_node_ray_leave,
    .ray_button = zns_board_node_ray_button,
};

static void
zns_board_handle_zn_board_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_board *self =
      zn_container_of(listener, self, zn_board_destroy_listener);

  zns_board_destroy(self);
}

struct zns_board *
zns_board_create(struct zn_board *zn_board)
{
  struct zns_board *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zn_board = zn_board;
  self->node = zns_node_create(server->shell->root, self, &node_implementation);
  if (self->node == NULL) {
    zn_error("Failed to create zns_node");
    goto err_free;
  }

  self->zn_board_destroy_listener.notify = zns_board_handle_zn_board_destroy;
  wl_signal_add(&zn_board->events.destroy, &self->zn_board_destroy_listener);

  wl_signal_init(&self->events.destroy);
  wl_list_init(&self->seat_capsule_link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zns_board_destroy(struct zns_board *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->zn_board_destroy_listener.link);
  wl_list_remove(&self->seat_capsule_link);
  zns_node_destroy(self->node);
  free(self);
}
