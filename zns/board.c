#include "board.h"

#include <cglm/quat.h>
#include <linux/input.h>
#include <zen-common.h>
#include <zgnr/intersection.h>

#include "ray-grab/board-move.h"
#include "shell.h"
#include "zen/server.h"

static void zns_board_destroy(struct zns_board *self);

float
zns_board_ray_cast(
    struct zns_board *self, vec3 origin, vec3 direction, float *u, float *v)
{
  float x = self->zn_board->geometry.size[0] / 2.f;
  float y = self->zn_board->geometry.size[1] / 2.f;
  vec3 v0 = {-x, -y, 0};
  vec3 v1 = {+x, -y, 0};
  vec3 v2 = {-x, +y, 0};

  glm_quat_rotatev(self->zn_board->geometry.quaternion, v0, v0);
  glm_quat_rotatev(self->zn_board->geometry.quaternion, v1, v1);
  glm_quat_rotatev(self->zn_board->geometry.quaternion, v2, v2);

  glm_vec3_add(self->zn_board->geometry.center, v0, v0);
  glm_vec3_add(self->zn_board->geometry.center, v1, v1);
  glm_vec3_add(self->zn_board->geometry.center, v2, v2);

  return zgnr_intersection_ray_parallelogram(
      origin, direction, v0, v1, v2, u, v, false);
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

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
static bool
zns_board_node_ray_motion(void *user_data, vec3 origin, vec3 direction,
    uint32_t time_msec, mat4 transform)
{
  return true;
}

static bool
zns_board_node_ray_enter(void *user_data, uint32_t serial, vec3 origin,
    vec3 direction, mat4 transform)
{
  return true;
}

static bool
zns_board_node_ray_leave(void *user_data, uint32_t serial, mat4 transform)
{
  return true;
}
#pragma GCC diagnostic pop

static bool
zns_board_node_ray_button(void *user_data, uint32_t serial, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state, mat4 transform)
{
  UNUSED(serial);
  UNUSED(time_msec);
  UNUSED(transform);  // must be identity matrix

  struct zns_board *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  if (state == ZGN_RAY_BUTTON_STATE_PRESSED && button == BTN_LEFT) {
    struct zns_board_move_ray_grab *board_move_grab;
    board_move_grab = zns_board_move_ray_grab_create(self);
    if (board_move_grab) {
      zn_ray_start_grab(server->scene->ray, &board_move_grab->base);
    }
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
  zns_node_destroy(self->node);
  free(self);
}
