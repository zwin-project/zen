#include "bounded.h"

#include <cglm/affine.h>
#include <cglm/mat4.h>
#include <cglm/quat.h>
#include <zen-common.h>
#include <zgnr/intersection.h>

#include "ray-grab/default.h"
#include "ray-grab/move.h"
#include "zen/scene/virtual-object.h"
#include "zen/server.h"

static void zns_bounded_destroy(struct zns_bounded* self);

float
zns_bounded_ray_cast(struct zns_bounded* self, struct zn_ray* ray)
{
  struct zn_virtual_object* zn_virtual_object =
      self->zgnr_bounded->virtual_object->user_data;
  float outer_distance, distance;

  if (!self->zgnr_bounded->current.region) return FLT_MAX;

  outer_distance = zgnr_intersection_ray_obb(ray->origin, ray->direction,
      self->zgnr_bounded->current.half_size, zn_virtual_object->model_matrix);

  if (outer_distance == FLT_MAX) return FLT_MAX;

  distance = zgnr_region_node_ray_cast(self->zgnr_bounded->current.region,
      zn_virtual_object->model_matrix, ray->origin, ray->direction);

  if (distance == FLT_MAX) return FLT_MAX;

  return distance < outer_distance ? outer_distance : distance;
}

static void
zns_bounded_handle_move(struct wl_listener* listener, void* data)
{
  struct zns_bounded* self = zn_container_of(listener, self, move_listener);
  struct zn_server* server = zn_server_get_singleton();
  struct zn_ray* ray = server->input_manager->seat->ray;
  struct zgnr_bounded_move_event* event = data;
  struct zns_default_ray_grab* grab = zns_default_ray_grab_get(ray->grab);
  struct zns_move_ray_grab* move_grab;

  if (grab == NULL || grab->focus != self ||
      grab->button_state != ZGN_RAY_BUTTON_STATE_PRESSED ||
      grab->last_button_serial != event->serial)
    return;

  move_grab = zns_move_ray_grab_create(self);
  zn_ray_start_grab(grab->base.ray, &move_grab->base);
}

static void
zns_bounded_handle_zgnr_bounded_destroy(
    struct wl_listener* listener, void* data)
{
  UNUSED(data);

  struct zns_bounded* self =
      zn_container_of(listener, self, zgnr_bounded_destroy_listener);

  zns_bounded_destroy(self);
}

struct zns_bounded*
zns_bounded_create(struct zgnr_bounded* zgnr_bounded)
{
  struct zns_bounded* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->zgnr_bounded = zgnr_bounded;
  wl_list_init(&self->link);

  self->zgnr_bounded_destroy_listener.notify =
      zns_bounded_handle_zgnr_bounded_destroy;
  wl_signal_add(
      &zgnr_bounded->events.destroy, &self->zgnr_bounded_destroy_listener);

  self->move_listener.notify = zns_bounded_handle_move;
  wl_signal_add(&zgnr_bounded->events.move, &self->move_listener);

  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

static void
zns_bounded_destroy(struct zns_bounded* self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->zgnr_bounded_destroy_listener.link);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
