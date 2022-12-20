#include "down.h"

#include <cglm/mat4.h>
#include <zen-common.h>

#include "node.h"
#include "shell.h"
#include "zen/appearance/ray.h"
#include "zen/server.h"

static void zns_down_ray_grab_destroy(struct zns_down_ray_grab *self);

static void
zns_down_ray_grab_motion_relative(struct zn_ray_grab *grab_base, vec3 origin,
    float polar, float azimuthal, uint32_t time_msec)
{
  struct zns_down_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zns_node *node;
  struct zn_server *server = zn_server_get_singleton();

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

  float distance = FLT_MAX;
  mat4 identity = GLM_MAT4_IDENTITY_INIT;
  node = zns_node_ray_cast(server->shell->root, self->base.ray->origin,
      self->base.ray->direction, identity, &distance);

  if (node == self->node) zn_ray_set_length(self->base.ray, distance);

  zna_ray_commit(self->base.ray->appearance);

  zns_node_ray_motion(
      self->node, self->base.ray->origin, self->base.ray->direction, time_msec);
}

static void
zns_down_ray_grab_button(struct zn_ray_grab *grab_base, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state)
{
  struct zns_down_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_server *server = zn_server_get_singleton();

  zns_node_ray_button(self->node, time_msec, button, state);

  if (server->input_manager->seat->pressing_button_count == 0 &&
      state == ZGN_RAY_BUTTON_STATE_RELEASED) {
    zn_ray_end_grab(self->base.ray);
  }
}

static void
zns_down_ray_grab_rebase(struct zn_ray_grab *grab_base)
{
  UNUSED(grab_base);
}

static void
zns_down_ray_grab_cancel(struct zn_ray_grab *grab_base)
{
  struct zns_down_ray_grab *self = zn_container_of(grab_base, self, base);
  zns_down_ray_grab_destroy(self);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_down_ray_grab_motion_relative,
    .button = zns_down_ray_grab_button,
    .rebase = zns_down_ray_grab_rebase,
    .cancel = zns_down_ray_grab_cancel,
};

static void
zns_down_ray_grab_handle_node_destroy(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_down_ray_grab *self =
      zn_container_of(listener, self, node_destroy_listener);
  zn_ray_end_grab(self->base.ray);
}

static struct zns_down_ray_grab *
zns_down_ray_grab_create(struct zns_node *node)
{
  struct zns_down_ray_grab *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->base.impl = &implementation;

  self->node = node;

  self->node_destroy_listener.notify = zns_down_ray_grab_handle_node_destroy;
  wl_signal_add(&node->events.destroy, &self->node_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zns_down_ray_grab_destroy(struct zns_down_ray_grab *self)
{
  wl_list_remove(&self->node_destroy_listener.link);
  free(self);
}

void
zns_down_ray_grab_start(struct zn_ray *ray, struct zns_node *node)
{
  struct zns_down_ray_grab *self = zns_down_ray_grab_create(node);
  if (!self) {
    zn_error("Failed to create zns_down_ray_grab");
    return;
  }

  zn_ray_start_grab(ray, &self->base);
}
