#include "bounded.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <zen-common.h>
#include <zgnr/intersection.h>

#include "ray-grab/default.h"
#include "ray-grab/move.h"
#include "seat-capsule.h"
#include "shell.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

static void zns_bounded_destroy(struct zns_bounded *self);

bool
zns_bounded_node_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_bounded *self = user_data;
  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_bounded->virtual_object->user_data;
  float outer_distance, inner_distance, intersected_distance;

  if (!self->zgnr_bounded->current.region) return false;

  outer_distance = zgnr_intersection_ray_obb(origin, direction,
      self->zgnr_bounded->current.half_size, zn_virtual_object->model_matrix);

  if (outer_distance == FLT_MAX || outer_distance > *distance) return false;

  inner_distance = zgnr_region_node_ray_cast(self->zgnr_bounded->current.region,
      zn_virtual_object->model_matrix, origin, direction);

  if (inner_distance == FLT_MAX) return false;

  intersected_distance =
      inner_distance < outer_distance ? outer_distance : inner_distance;

  if (intersected_distance > *distance) return false;

  *distance = intersected_distance;

  return true;
}

static bool
zns_bounded_node_ray_motion(void *user_data, vec3 origin, vec3 direction,
    uint32_t time_msec, mat4 transform)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_bounded *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_bounded->virtual_object->user_data;

  vec3 local_origin, local_direction, tip, local_tip;
  glm_vec3_add(origin, direction, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);

  zgnr_seat_ray_send_motion(server->input_manager->seat->zgnr_seat, time_msec,
      local_origin, local_direction);

  return true;
}

static bool
zns_bounded_node_ray_enter(
    void *user_data, vec3 origin, vec3 direction, mat4 transform)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_bounded *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_bounded->virtual_object->user_data;

  vec3 local_origin, local_direction, tip, local_tip;
  glm_vec3_add(origin, direction, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);

  zgnr_seat_ray_enter(server->input_manager->seat->zgnr_seat,
      self->zgnr_bounded->virtual_object, local_origin, local_direction);

  return true;
}

static bool
zns_bounded_node_ray_leave(void *user_data, mat4 transform)
{
  UNUSED(user_data);
  UNUSED(transform);  // must be identity matrix
  struct zn_server *server = zn_server_get_singleton();

  zgnr_seat_ray_clear_focus(server->input_manager->seat->zgnr_seat);

  return true;
}

static bool
zns_bounded_node_ray_button(void *user_data, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state, mat4 transform)
{
  UNUSED(user_data);
  UNUSED(transform);  // must be identity matrix
  struct zn_server *server = zn_server_get_singleton();

  zgnr_seat_ray_send_button(
      server->input_manager->seat->zgnr_seat, time_msec, button, state);

  return true;
}

static const struct zns_node_interface node_implementation = {
    .ray_cast = zns_bounded_node_ray_cast,
    .ray_motion = zns_bounded_node_ray_motion,
    .ray_enter = zns_bounded_node_ray_enter,
    .ray_leave = zns_bounded_node_ray_leave,
    .ray_button = zns_bounded_node_ray_button,
};

static void
zns_bounded_handle_mapped(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_bounded *self = zn_container_of(listener, self, mapped_listener);
  struct zn_server *server = zn_server_get_singleton();

  zns_seat_capsule_add_bounded(server->shell->seat_capsule, self);
}

static void
zns_bounded_handle_move(struct wl_listener *listener, void *data)
{
  struct zns_bounded *self = zn_container_of(listener, self, move_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zgnr_seat *zgnr_seat = server->input_manager->seat->zgnr_seat;
  struct zn_ray *ray = server->scene->ray;
  struct zgnr_bounded_move_event *event = data;

  // TODO: Check that ray button is pressing
  if (zgnr_seat->ray_state.focus_virtual_object !=
          self->zgnr_bounded->virtual_object ||
      zgnr_seat->ray_state.last_button_serial != event->serial)
    return;

  zns_move_ray_grab_start(ray, self);
}

static void
zns_bounded_handle_zgnr_bounded_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_bounded *self =
      zn_container_of(listener, self, zgnr_bounded_destroy_listener);

  zns_bounded_destroy(self);
}

struct zns_bounded *
zns_bounded_create(struct zgnr_bounded *zgnr_bounded)
{
  struct zns_bounded *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->node = zns_node_create(server->shell->root, self, &node_implementation);
  if (self->node == NULL) {
    zn_error("Failed to create zns_node");
    goto err_free;
  }

  self->zgnr_bounded = zgnr_bounded;
  wl_list_init(&self->link);
  wl_list_init(&self->seat_capsule_link);

  self->zgnr_bounded_destroy_listener.notify =
      zns_bounded_handle_zgnr_bounded_destroy;
  wl_signal_add(
      &zgnr_bounded->events.destroy, &self->zgnr_bounded_destroy_listener);

  self->move_listener.notify = zns_bounded_handle_move;
  wl_signal_add(&zgnr_bounded->events.move, &self->move_listener);

  self->mapped_listener.notify = zns_bounded_handle_mapped;
  wl_signal_add(&zgnr_bounded->events.mapped, &self->mapped_listener);

  wl_signal_init(&self->events.destroy);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zns_bounded_destroy(struct zns_bounded *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->link);
  wl_list_remove(&self->seat_capsule_link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->mapped_listener.link);
  wl_list_remove(&self->zgnr_bounded_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  zns_node_destroy(self->node);
  free(self);
}
