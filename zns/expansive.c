#include "expansive.h"

#include <zen-common.h>

#include "shell.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

static void zns_expansive_destroy(struct zns_expansive *self);

static bool
zns_expansive_node_ray_cast(void *user_data, vec3 origin, vec3 direction,
    mat4 transform, float *distance)
{
  UNUSED(transform);  // must be identity matrix
  struct zns_expansive *self = user_data;
  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_expansive->virtual_object->user_data;
  float intersected_distance;

  if (!self->zgnr_expansive->current.region) return false;

  intersected_distance =
      zgnr_region_node_ray_cast(self->zgnr_expansive->current.region,
          zn_virtual_object->model_matrix, origin, direction);

  if (intersected_distance >= *distance) return false;

  *distance = intersected_distance;

  return true;
}

static bool
zns_expansive_node_ray_motion(
    void *user_data, vec3 origin, vec3 direction, uint32_t time_msec)
{
  struct zns_expansive *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_expansive->virtual_object->user_data;

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
zns_expansive_node_ray_enter(void *user_data, vec3 origin, vec3 direction)
{
  struct zns_expansive *self = user_data;
  struct zn_server *server = zn_server_get_singleton();

  struct zn_virtual_object *zn_virtual_object =
      self->zgnr_expansive->virtual_object->user_data;

  vec3 local_origin, local_direction, tip, local_tip;
  glm_vec3_add(origin, direction, tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(zn_virtual_object->model_invert, origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);

  zgnr_seat_ray_enter(server->input_manager->seat->zgnr_seat,
      self->zgnr_expansive->virtual_object, local_origin, local_direction);

  return true;
}

static bool
zns_expansive_node_ray_leave(void *user_data)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  zgnr_seat_ray_clear_focus(server->input_manager->seat->zgnr_seat);

  return true;
}

static bool
zns_expansive_node_ray_button(void *user_data, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  enum zgn_ray_button_state zgn_state;
  if (state == WLR_BUTTON_PRESSED) {
    zgn_state = ZGN_RAY_BUTTON_STATE_PRESSED;
  } else {
    zgn_state = ZGN_RAY_BUTTON_STATE_RELEASED;
  }

  zgnr_seat_ray_send_button(
      server->input_manager->seat->zgnr_seat, time_msec, button, zgn_state);

  return true;
}

static bool
zns_expansive_node_ray_axis(void *user_data, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();
  enum zgn_ray_axis axis;
  enum zgn_ray_axis_source zgn_axis_source;

  switch (orientation) {
    case WLR_AXIS_ORIENTATION_VERTICAL:
      axis = ZGN_RAY_AXIS_VERTICAL_SCROLL;
      break;

    case WLR_AXIS_ORIENTATION_HORIZONTAL:
      axis = ZGN_RAY_AXIS_HORIZONTAL_SCROLL;
      break;
  }

  switch (source) {
    case WLR_AXIS_SOURCE_WHEEL:
      zgn_axis_source = ZGN_RAY_AXIS_SOURCE_WHEEL;
      break;

    case WLR_AXIS_SOURCE_FINGER:
      zgn_axis_source = ZGN_RAY_AXIS_SOURCE_FINGER;
      break;

    case WLR_AXIS_SOURCE_CONTINUOUS:
      zgn_axis_source = ZGN_RAY_AXIS_SOURCE_CONTINUOUS;
      break;

    case WLR_AXIS_SOURCE_WHEEL_TILT:
      zgn_axis_source = ZGN_RAY_AXIS_SOURCE_WHEEL_TILT;
      break;
  }

  zgnr_seat_ray_send_axis(server->input_manager->seat->zgnr_seat, time_msec,
      axis, delta, delta_discrete, zgn_axis_source);

  return true;
}

static bool
zns_expansive_node_ray_frame(void *user_data)
{
  UNUSED(user_data);
  struct zn_server *server = zn_server_get_singleton();

  zgnr_seat_ray_send_frame(server->input_manager->seat->zgnr_seat);

  return true;
}

static const struct zns_node_interface node_implementation = {
    .ray_cast = zns_expansive_node_ray_cast,
    .ray_motion = zns_expansive_node_ray_motion,
    .ray_enter = zns_expansive_node_ray_enter,
    .ray_leave = zns_expansive_node_ray_leave,
    .ray_button = zns_expansive_node_ray_button,
    .ray_axis = zns_expansive_node_ray_axis,
    .ray_frame = zns_expansive_node_ray_frame,
};

static void
zns_expansive_handle_zgnr_expansive_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zns_expansive *self =
      zn_container_of(listener, self, zgnr_expansive_destroy_listener);

  zns_expansive_destroy(self);
}

struct zns_expansive *
zns_expansive_create(struct zgnr_expansive *zgnr_expansive)
{
  struct zns_expansive *self;
  struct zn_server *server = zn_server_get_singleton();

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory ");
    goto err;
  }

  self->node = zns_node_create(server->shell->root, self, &node_implementation);
  if (self->node == NULL) {
    zn_error("Failed to create zns_node");
    goto err_free;
  }

  self->zgnr_expansive = zgnr_expansive;

  self->zgnr_expansive_destroy_listener.notify =
      zns_expansive_handle_zgnr_expansive_destroy;
  wl_signal_add(
      &zgnr_expansive->events.destroy, &self->zgnr_expansive_destroy_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zns_expansive_destroy(struct zns_expansive *self)
{
  wl_list_remove(&self->zgnr_expansive_destroy_listener.link);
  zns_node_destroy(self->node);
  free(self);
}
