#include "node.h"

#include <cglm/mat4.h>
#include <zen-common.h>

struct zns_node *
zns_node_ray_cast(struct zns_node *self, vec3 origin, vec3 direction,
    mat4 parent_transform, float *distance)
{
  mat4 transform;
  glm_mat4_mul(self->transform, parent_transform, transform);

  struct zns_node *nearest_node = NULL;
  if (self->implementation->ray_cast(
          self->user_data, origin, direction, transform, distance)) {
    nearest_node = self;
  }

  struct zns_node *child;
  wl_list_for_each (child, &self->children, link) {
    struct zns_node *node =
        zns_node_ray_cast(child, origin, direction, transform, distance);
    if (node) nearest_node = node;
  }

  return nearest_node;
}

void
zns_node_ray_motion(
    struct zns_node *self, vec3 origin, vec3 direction, uint32_t time_msec)
{
  if (!self->implementation->ray_motion(
          self->user_data, origin, direction, time_msec) &&
      self->parent) {
    zns_node_ray_motion(self->parent, origin, direction, time_msec);
  }
}

void
zns_node_ray_enter(struct zns_node *self, vec3 origin, vec3 direction)
{
  if (!self->implementation->ray_enter(self->user_data, origin, direction) &&
      self->parent) {
    zns_node_ray_enter(self->parent, origin, direction);
  }
}

void
zns_node_ray_leave(struct zns_node *self)
{
  if (!self->implementation->ray_leave(self->user_data) && self->parent) {
    zns_node_ray_leave(self->parent);
  }
}

void
zns_node_ray_button(struct zns_node *self, uint32_t time_msec, uint32_t button,
    enum wlr_button_state state)
{
  if (!self->implementation->ray_button(
          self->user_data, time_msec, button, state) &&
      self->parent) {
    zns_node_ray_button(self->parent, time_msec, button, state);
  }
}

void
zns_node_ray_axis(struct zns_node *self, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  if (!self->implementation->ray_axis(self->user_data, time_msec, source,
          orientation, delta, delta_discrete) &&
      self->parent) {
    zns_node_ray_axis(
        self->parent, time_msec, source, orientation, delta, delta_discrete);
  }
}

void
zns_node_ray_frame(struct zns_node *self)
{
  if (!self->implementation->ray_frame(self->user_data) && self->parent) {
    zns_node_ray_frame(self->parent);
  }
}

struct zns_node *
zns_node_create(struct zns_node *parent, void *user_data,
    const struct zns_node_interface *implementation)
{
  struct zns_node *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->parent = parent;
  wl_list_init(&self->children);
  if (parent) {
    wl_list_insert(&parent->children, &self->link);
  } else {
    wl_list_init(&self->link);
  }
  self->user_data = user_data;
  self->implementation = implementation;

  wl_signal_init(&self->events.destroy);

  glm_mat4_identity(self->transform);

  return self;

err:
  return NULL;
}

void
zns_node_destroy(struct zns_node *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  wl_list_remove(&self->link);
  wl_list_remove(&self->children);
  free(self);
}
