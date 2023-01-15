#include "zns/bounded.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <zen-common.h>
#include <zwnr/intersection.h>

#include "zen/server.h"
#include "zen/virtual-object.h"
#include "zns/appearance/bounded.h"
#include "zns/bounded-nameplate.h"
#include "zns/ray-grab/default.h"
#include "zns/ray-grab/move.h"
#include "zns/seat-capsule.h"
#include "zns/shell.h"

static void zns_bounded_destroy(struct zns_bounded *self);

static bool
zns_bounded_node_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_bounded *self = user_data;
  struct zn_virtual_object *zn_virtual_object =
      self->zwnr_bounded->virtual_object->user_data;
  float outer_distance, inner_distance, intersected_distance;

  if (!self->zwnr_bounded->current.region) return false;

  outer_distance = zwnr_intersection_ray_obb(origin, direction,
      self->zwnr_bounded->current.half_size, zn_virtual_object->model_matrix);

  if (outer_distance == FLT_MAX || outer_distance > *distance) return false;

  inner_distance = zwnr_region_node_ray_cast(self->zwnr_bounded->current.region,
      zn_virtual_object->model_matrix, origin, direction);

  if (inner_distance == FLT_MAX) return false;

  intersected_distance =
      inner_distance < outer_distance ? outer_distance : inner_distance;

  if (intersected_distance > *distance) return false;

  *distance = intersected_distance;

  return true;
}

static bool
zns_bounded_node_ray_motion(
    void *user_data, vec3 origin, vec3 direction, uint32_t time_msec)
{
  struct zns_bounded *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zwnr_bounded->virtual_object->user_data;

  vec3 local_origin, local_direction, tip, local_tip;
  glm_vec3_add(origin, direction, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);

  zwnr_seat_ray_send_motion(server->input_manager->seat->zwnr_seat, time_msec,
      local_origin, local_direction);

  return true;
}

static bool
zns_bounded_node_ray_enter(void *user_data, vec3 origin, vec3 direction)
{
  struct zns_bounded *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zwnr_bounded->virtual_object->user_data;

  vec3 local_origin, local_direction, tip, local_tip;
  glm_vec3_add(origin, direction, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);

  zwnr_seat_ray_enter(server->input_manager->seat->zwnr_seat,
      self->zwnr_bounded->virtual_object, local_origin, local_direction);

  return true;
}

static bool
zns_bounded_node_ray_leave(void *user_data)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  zwnr_seat_ray_clear_focus(server->input_manager->seat->zwnr_seat);

  return true;
}

static bool
zns_bounded_node_ray_button(void *user_data, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  enum zwn_ray_button_state zwn_state;
  if (state == WLR_BUTTON_PRESSED) {
    zwn_state = ZWN_RAY_BUTTON_STATE_PRESSED;
  } else {
    zwn_state = ZWN_RAY_BUTTON_STATE_RELEASED;
  }

  zwnr_seat_ray_send_button(
      server->input_manager->seat->zwnr_seat, time_msec, button, zwn_state);

  return true;
}

static bool
zns_bounded_node_ray_axis(void *user_data, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();
  enum zwn_ray_axis axis;
  enum zwn_ray_axis_source zwn_axis_source;

  switch (orientation) {
    case WLR_AXIS_ORIENTATION_VERTICAL:
      axis = ZWN_RAY_AXIS_VERTICAL_SCROLL;
      break;

    case WLR_AXIS_ORIENTATION_HORIZONTAL:
      axis = ZWN_RAY_AXIS_HORIZONTAL_SCROLL;
      break;
  }

  switch (source) {
    case WLR_AXIS_SOURCE_WHEEL:
      zwn_axis_source = ZWN_RAY_AXIS_SOURCE_WHEEL;
      break;

    case WLR_AXIS_SOURCE_FINGER:
      zwn_axis_source = ZWN_RAY_AXIS_SOURCE_FINGER;
      break;

    case WLR_AXIS_SOURCE_CONTINUOUS:
      zwn_axis_source = ZWN_RAY_AXIS_SOURCE_CONTINUOUS;
      break;

    case WLR_AXIS_SOURCE_WHEEL_TILT:
      zwn_axis_source = ZWN_RAY_AXIS_SOURCE_WHEEL_TILT;
      break;
  }

  zwnr_seat_ray_send_axis(server->input_manager->seat->zwnr_seat, time_msec,
      axis, delta, delta_discrete, zwn_axis_source);

  return true;
}

static bool
zns_bounded_node_ray_frame(void *user_data)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  zwnr_seat_ray_send_frame(server->input_manager->seat->zwnr_seat);

  return true;
}

static const struct zns_node_interface node_implementation = {
    .ray_cast = zns_bounded_node_ray_cast,
    .ray_motion = zns_bounded_node_ray_motion,
    .ray_enter = zns_bounded_node_ray_enter,
    .ray_leave = zns_bounded_node_ray_leave,
    .ray_button = zns_bounded_node_ray_button,
    .ray_axis = zns_bounded_node_ray_axis,
    .ray_frame = zns_bounded_node_ray_frame,
};

static void
zns_bounded_handle_mapped(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_bounded *self = zn_container_of(listener, self, mapped_listener);
  struct zn_server *server = zn_server_get_singleton();

  zns_seat_capsule_add_bounded(server->shell->seat_capsule, self);

  zna_bounded_commit(self->appearance, ZNA_BOUNDED_DAMAGE_GEOMETRY);
}

static void
zns_bounded_handle_move(struct wl_listener *listener, void *data)
{
  struct zns_bounded *self = zn_container_of(listener, self, move_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zwnr_seat *zwnr_seat = server->input_manager->seat->zwnr_seat;
  struct zn_ray *ray = server->scene->ray;
  struct zwnr_bounded_move_event *event = data;

  // TODO: Check that ray button is pressing
  if (zwnr_seat->ray_state.focus_virtual_object !=
          self->zwnr_bounded->virtual_object ||
      zwnr_seat->ray_state.last_button_serial != event->serial)
    return;

  zns_move_ray_grab_start(ray, self);
}

static void
zns_bounded_handle_zwnr_bounded_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_bounded *self =
      zn_container_of(listener, self, zwnr_bounded_destroy_listener);

  zns_bounded_destroy(self);
}

static void
zns_bounded_handle_virtual_object_commit(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_bounded *self = zn_container_of(listener, self, commit_listener);

  if (self->zwnr_bounded->current.damage & ZWNR_BOUNDED_DAMAGE_TITLE) {
    self->zwnr_bounded->current.damage &= (~ZWNR_BOUNDED_DAMAGE_TITLE);
    zna_bounded_commit(self->appearance, ZNA_BOUNDED_DAMAGE_NAMEPLATE_TEXTURE);
  }
}

struct zns_bounded *
zns_bounded_create(struct zwnr_bounded *zwnr_bounded)
{
  struct zns_bounded *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->node = zns_node_create(
      server->shell->root, self, &node_implementation, ZNS_NODE_BOUNDED);
  if (self->node == NULL) {
    zn_error("Failed to create zns_node");
    goto err_free;
  }

  self->zwnr_bounded = zwnr_bounded;
  wl_list_init(&self->link);
  wl_list_init(&self->seat_capsule_link);

  self->nameplate = zns_bounded_nameplate_create(self);
  if (self->nameplate == NULL) {
    zn_error("Failed to create a bounded nameplate");
    goto err_node;
  }

  self->appearance = zna_bounded_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    zn_error("Failed to create a zna_bounded");
    goto err_nameplate;
  }

  self->zwnr_bounded_destroy_listener.notify =
      zns_bounded_handle_zwnr_bounded_destroy;
  wl_signal_add(
      &zwnr_bounded->events.destroy, &self->zwnr_bounded_destroy_listener);

  self->move_listener.notify = zns_bounded_handle_move;
  wl_signal_add(&zwnr_bounded->events.move, &self->move_listener);

  self->mapped_listener.notify = zns_bounded_handle_mapped;
  wl_signal_add(&zwnr_bounded->events.mapped, &self->mapped_listener);

  self->commit_listener.notify = zns_bounded_handle_virtual_object_commit;
  wl_signal_add(
      &zwnr_bounded->virtual_object->events.committed, &self->commit_listener);

  wl_signal_init(&self->events.destroy);

  return self;

err_nameplate:
  zns_bounded_nameplate_destroy(self->nameplate);

err_node:
  zns_node_destroy(self->node);

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
  wl_list_remove(&self->zwnr_bounded_destroy_listener.link);
  wl_list_remove(&self->commit_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  zns_bounded_nameplate_destroy(self->nameplate);
  zna_bounded_destroy(self->appearance);
  zns_node_destroy(self->node);
  free(self);
}
