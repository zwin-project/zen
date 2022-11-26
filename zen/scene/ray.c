#include "zen/scene/ray.h"

#include "zen-common.h"
#include "zen/appearance/scene/ray.h"
#include "zen/server.h"

void
zn_ray_get_tip(struct zn_ray* self, float length, vec3 tip)
{
  tip[1] = length * cosf32(self->angle.polar);
  float r = length * sinf32(self->angle.polar);
  tip[0] = r * cos(self->angle.azimuthal);
  tip[2] = -r * sin(self->angle.azimuthal);

  glm_vec3_add(tip, self->origin, tip);
}

void
zn_ray_move(struct zn_ray* self, float polar, float azimuthal)
{
  self->angle.polar = polar;
  self->angle.azimuthal = azimuthal;

  wl_signal_emit(&self->events.motion, NULL);
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

  glm_vec3_copy(initial_origin, self->origin);
  self->angle.polar = GLM_PI / 3;
  self->angle.azimuthal = GLM_PI * 0.5;

  wl_signal_init(&self->events.destroy);
  wl_signal_init(&self->events.motion);

  self->appearance = zna_ray_create(self, server->appearance_system);
  if (self->appearance == NULL) {
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_ray_destroy(struct zn_ray* self)
{
  wl_signal_emit(&self->events.destroy, NULL);

  zna_ray_destroy(self->appearance);
  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->events.motion.listener_list);
  free(self);
}
