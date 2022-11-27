#include "zen/ray.h"

#include <cglm/mat4.h>
#include <cglm/vec3.h>

#include "zen-common.h"
#include "zen/appearance/ray.h"
#include "zen/scene/virtual-object.h"
#include "zen/server.h"

#define DEFAULT_RAY_LENGTH 1

void
zn_ray_get_tip(struct zn_ray* self, vec3 tip)
{
  glm_vec3_scale_as(self->direction, self->length, tip);
  glm_vec3_add(tip, self->origin, tip);
}

void
zn_ray_get_local_origin_direction(struct zn_ray* self,
    struct zn_virtual_object* virtual_object, vec3 local_origin,
    vec3 local_direction)
{
  vec3 tip, local_tip;
  zn_ray_get_tip(self, tip);
  glm_mat4_mulv3(virtual_object->model_invert, tip, 1, local_tip);
  glm_mat4_mulv3(virtual_object->model_invert, self->origin, 1, local_origin);
  glm_vec3_sub(local_tip, local_origin, local_direction);
  glm_vec3_scale_as(local_direction, 1, local_direction);
}

void
zn_ray_set_length(struct zn_ray* self, float length)
{
  self->length = length;
}

void
zn_ray_move(struct zn_ray* self, vec3 origin, float polar, float azimuthal)
{
  self->angle.polar = polar;
  self->angle.azimuthal = azimuthal;
  glm_vec3_copy(origin, self->origin);

  float r = sinf(self->angle.polar);
  self->direction[0] = r * cosf(self->angle.azimuthal);
  self->direction[1] = cosf(self->angle.polar);
  self->direction[2] = -r * sinf(self->angle.azimuthal);
}

void
zn_ray_start_grab(struct zn_ray* self, struct zn_ray_grab* grab)
{
  struct zn_server* server = zn_server_get_singleton();
  struct zn_ray_grab* default_grab = zn_shell_get_default_grab(server->shell);

  if (!zn_assert(
          self->grab == default_grab, "Non-default grab already exists")) {
    return;
  }

  self->grab->interface->cancel(self->grab);

  self->grab = grab;
  self->grab->ray = self;
}

void
zn_ray_end_grab(struct zn_ray* self)
{
  struct zn_server* server = zn_server_get_singleton();

  self->grab->interface->cancel(self->grab);

  self->grab = zn_shell_get_default_grab(server->shell);
  self->grab->ray = self;
}

struct zn_ray*
zn_ray_create(void)
{
  struct zn_ray* self;
  struct zn_server* server = zn_server_get_singleton();
  vec3 initial_origin = {0.3f, 0.7f, 0.0f};

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->grab = zn_shell_get_default_grab(server->shell);
  self->grab->ray = self;

  wl_signal_init(&self->events.destroy);

  self->appearance = zna_ray_create(self, server->appearance_system);
  if (self->appearance == NULL) {
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
zn_ray_destroy(struct zn_ray* self)
{
  zn_ray_end_grab(self);

  wl_signal_emit(&self->events.destroy, NULL);

  zna_ray_destroy(self->appearance);
  wl_list_remove(&self->events.destroy.listener_list);
  free(self);
}
