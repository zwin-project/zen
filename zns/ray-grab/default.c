#include "default.h"

#include <cglm/vec3.h>
#include <zen-common.h>
#include <zigen-protocol.h>

#include "bounded.h"
#include "shell.h"
#include "zen/appearance/ray.h"
#include "zen/server.h"
#include "zen/virtual-object.h"

/**
 * @param bounded is nullable
 */
static void
zns_default_ray_grab_focus(
    struct zns_default_ray_grab *self, struct zns_node *node)
{
  if (self->focus == node) return;

  struct zn_server *server = zn_server_get_singleton();

  if (self->focus) {
    wl_list_remove(&self->focus_destroy_listener.link);
    wl_list_init(&self->focus_destroy_listener.link);

    uint32_t serial = wl_display_next_serial(server->display);

    zns_node_ray_leave(self->focus, serial);
  }

  if (node) {
    wl_signal_add(&node->events.destroy, &self->focus_destroy_listener);

    uint32_t serial = wl_display_next_serial(server->display);

    zns_node_ray_enter(
        node, serial, self->base.ray->origin, self->base.ray->direction);
  }

  self->focus = node;
}

static void
zns_default_ray_grab_motion_relative(struct zn_ray_grab *grab_base, vec3 origin,
    float polar, float azimuthal, uint32_t time_msec)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zns_node *node;

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
  node = zns_node_ray_cast(self->shell->root, self->base.ray->origin,
      self->base.ray->direction, identity, &distance);

  zn_ray_set_length(self->base.ray, node ? distance : DEFAULT_RAY_LENGTH);

  zna_ray_commit(self->base.ray->appearance);

  zns_default_ray_grab_focus(self, node);

  if (self->focus) {
    zns_node_ray_motion(self->focus, self->base.ray->origin,
        self->base.ray->direction, time_msec);
  }
}

static void
zns_default_ray_grab_button(struct zn_ray_grab *grab_base, uint32_t time_msec,
    uint32_t button, enum zgn_ray_button_state state)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_server *server = zn_server_get_singleton();

  self->button_state = state;
  self->last_button_serial = 0;

  if (self->focus == NULL) return;

  uint32_t serial = wl_display_next_serial(server->display);

  zns_node_ray_button(self->focus, serial, time_msec, button, state);

  self->last_button_serial = serial;
}

static void
zns_default_ray_grab_rebase(struct zn_ray_grab *grab_base)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zns_node *node;

  float distance = FLT_MAX;
  mat4 identity = GLM_MAT4_IDENTITY_INIT;
  node = zns_node_ray_cast(self->shell->root, self->base.ray->origin,
      self->base.ray->direction, identity, &distance);

  zn_ray_set_length(self->base.ray, node ? distance : DEFAULT_RAY_LENGTH);

  zna_ray_commit(self->base.ray->appearance);

  zns_default_ray_grab_focus(self, node);
}

static void
zns_default_ray_grab_cancel(struct zn_ray_grab *grab_base)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);
  zns_default_ray_grab_focus(self, NULL);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_default_ray_grab_motion_relative,
    .button = zns_default_ray_grab_button,
    .rebase = zns_default_ray_grab_rebase,
    .cancel = zns_default_ray_grab_cancel,
};

void
zns_default_ray_grab_handle_focus_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zns_default_ray_grab *self =
      zn_container_of(listener, self, focus_destroy_listener);

  zns_default_ray_grab_focus(self, NULL);
}

struct zns_default_ray_grab *
zns_default_ray_grab_get(struct zn_ray_grab *grab)
{
  if (grab->impl != &implementation) return NULL;
  struct zns_default_ray_grab *self;

  self = zn_container_of(grab, self, base);

  return self;
}

void
zns_default_ray_grab_init(
    struct zns_default_ray_grab *self, struct zn_shell *shell)
{
  self->base.impl = &implementation;
  self->shell = shell;
  self->focus = NULL;
  self->last_button_serial = 0;

  self->focus_destroy_listener.notify =
      zns_default_ray_grab_handle_focus_destroy;
  wl_list_init(&self->focus_destroy_listener.link);
}

void
zns_default_ray_grab_fini(struct zns_default_ray_grab *self)
{
  wl_list_remove(&self->focus_destroy_listener.link);
}
