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

static void
zns_node_get_accum_transform(struct zns_node *self, mat4 transform)
{
  struct zns_node *node = self;
  glm_mat4_identity(transform);
  while (node != NULL) {
    glm_mat4_mul(transform, node->transform, transform);
    node = node->parent;
  }
}

void
zns_node_ray_motion(
    struct zns_node *self, vec3 origin, vec3 direction, uint32_t time_msec)
{
  mat4 transform;
  zns_node_get_accum_transform(self, transform);

  if (!self->implementation->ray_motion(
          self->user_data, origin, direction, time_msec, transform) &&
      self->parent) {
    zns_node_ray_motion(self->parent, origin, direction, time_msec);
  }
}

void
zns_node_ray_enter(struct zns_node *self, vec3 origin, vec3 direction)
{
  mat4 transform;
  zns_node_get_accum_transform(self, transform);

  if (!self->implementation->ray_enter(
          self->user_data, origin, direction, transform) &&
      self->parent) {
    zns_node_ray_enter(self->parent, origin, direction);
  }
}

void
zns_node_ray_leave(struct zns_node *self)
{
  mat4 transform;
  zns_node_get_accum_transform(self, transform);

  if (!self->implementation->ray_leave(self->user_data, transform) &&
      self->parent) {
    zns_node_ray_leave(self->parent);
  }
}

void
zns_node_ray_button(struct zns_node *self, uint32_t time_msec, uint32_t button,
    enum zgn_ray_button_state state)
{
  mat4 transform;
  zns_node_get_accum_transform(self, transform);

  if (!self->implementation->ray_button(
          self->user_data, time_msec, button, state, transform) &&
      self->parent) {
    zns_node_ray_button(self->parent, time_msec, button, state);
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
