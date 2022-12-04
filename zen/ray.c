#include "zen/ray.h"

#include <cglm/mat4.h>
#include <cglm/vec3.h>

#include "zen-common.h"
#include "zen/appearance/ray.h"
#include "zen/server.h"
#include "zen/shell/shell.h"
#include "zen/virtual-object.h"

void
zn_ray_get_tip(struct zn_ray *self, vec3 tip)
{
  glm_vec3_scale_as(self->direction, self->length, tip);
  glm_vec3_add(tip, self->origin, tip);
}

void
zn_ray_set_length(struct zn_ray *self, float length)
{
  self->length = length;
}

void
zn_ray_move(struct zn_ray *self, vec3 origin, float polar, float azimuthal)
{
  self->angle.polar = polar;
  self->angle.azimuthal = azimuthal;
  glm_vec3_copy(origin, self->origin);

  float r = sinf(self->angle.polar);
  self->direction[0] = r * cosf(self->angle.azimuthal);
  self->direction[1] = cosf(self->angle.polar);
  self->direction[2] = -r * sinf(self->angle.azimuthal);
}

static bool
zn_ray_is_default_grab(struct zn_ray *self)
{
  if (self->grab == NULL || self->default_grab == NULL) return false;

  return self->grab->interface == self->default_grab->interface;
}

void
zn_ray_start_grab(struct zn_ray *self, struct zn_ray_grab *grab)
{
  if (!zn_assert(
          zn_ray_is_default_grab(self), "Non-default grab already exists")) {
    return;
  }

  self->grab->interface->cancel(self->grab);

  self->grab = grab;
  self->grab->ray = self;

  self->grab->interface->rebase(self->grab);
}

void
zn_ray_end_grab(struct zn_ray *self)
{
  self->grab->interface->cancel(self->grab);

  self->grab = self->default_grab;
  self->grab->ray = self;

  self->grab->interface->rebase(self->grab);
}

void
zn_ray_set_default_grab(struct zn_ray *self, struct zn_ray_grab *default_grab)
{
  if (!zn_assert(self->default_grab == NULL, "default grab already set")) {
    return;
  }
  self->default_grab = default_grab;
  self->grab = default_grab;
  self->grab->ray = self;
}

struct zn_ray *
zn_ray_create(void)
{
  struct zn_ray *self;
  struct zn_server *server = zn_server_get_singleton();
  vec3 initial_origin = {0.3f, 0.7f, 0.0f};

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->grab = NULL;
  self->default_grab = NULL;

  wl_signal_init(&self->events.destroy);

  self->appearance = zna_ray_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    zn_error("Failed to create a zna_ray");
    goto err_free;
  }

  zn_ray_move(self, initial_origin, GLM_PI / 3.0f, GLM_PI / 2.0f);
  zn_ray_set_length(self, DEFAULT_RAY_LENGTH);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ray_destroy_resources(struct zn_ray *self)
{
  if (self->grab) self->grab->interface->cancel(self->grab);
  self->grab = NULL;
}

void
zn_ray_destroy(struct zn_ray *self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  zna_ray_destroy(self->appearance);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
