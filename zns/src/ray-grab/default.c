#include "zns/ray-grab/default.h"

#include <cglm/vec3.h>
#include <zen-common.h>
#include <zwin-protocol.h>

#include "zen/appearance/ray.h"
#include "zen/server.h"
#include "zen/virtual-object.h"
#include "zns/bounded.h"
#include "zns/ray-grab/down.h"
#include "zns/shell.h"

/**
 * @param bounded is nullable
 */
static void
zns_default_ray_grab_focus(
    struct zns_default_ray_grab *self, struct zns_node *node)
{
  if (node) {
    zn_shell_ray_enter(
        self->shell, node, self->base.ray->origin, self->base.ray->direction);
  } else {
    zn_shell_ray_clear_focus(self->shell);
  }
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

  if (self->shell->ray_focus) {
    zns_node_ray_motion(self->shell->ray_focus, self->base.ray->origin,
        self->base.ray->direction, time_msec);
  }
}

static void
zns_default_ray_grab_button(struct zn_ray_grab *grab_base, uint32_t time_msec,
    uint32_t button, enum wlr_button_state state)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);
  struct zn_server *server = zn_server_get_singleton();

  self->button_state = state;

  if (self->shell->ray_focus == NULL) return;

  if (server->input_manager->seat->pressing_button_count == 1 &&
      state == WLR_BUTTON_PRESSED) {
    zns_down_ray_grab_start(self->base.ray, self->shell->ray_focus);
  }

  zns_node_ray_button(self->shell->ray_focus, time_msec, button, state);
}

static void
zns_default_ray_grab_axis(struct zn_ray_grab *grab_base, uint32_t time_msec,
    enum wlr_axis_source source, enum wlr_axis_orientation orientation,
    double delta, int32_t delta_discrete)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);

  if (self->shell->ray_focus == NULL) return;

  zns_node_ray_axis(self->shell->ray_focus, time_msec, source, orientation,
      delta, delta_discrete);
}

static void
zns_default_ray_grab_frame(struct zn_ray_grab *grab_base)
{
  struct zns_default_ray_grab *self = zn_container_of(grab_base, self, base);

  if (self->shell->ray_focus == NULL) return;

  zns_node_ray_frame(self->shell->ray_focus);
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
  UNUSED(grab_base);
}

static const struct zn_ray_grab_interface implementation = {
    .motion_relative = zns_default_ray_grab_motion_relative,
    .button = zns_default_ray_grab_button,
    .axis = zns_default_ray_grab_axis,
    .frame = zns_default_ray_grab_frame,
    .rebase = zns_default_ray_grab_rebase,
    .cancel = zns_default_ray_grab_cancel,
};

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
}

void
zns_default_ray_grab_fini(struct zns_default_ray_grab *self)
{
  UNUSED(self);
}
