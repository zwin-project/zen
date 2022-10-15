#include "zen/scene/ray.h"

#include "zen-common.h"

struct zn_ray*
zn_ray_create(void)
{
  struct zn_ray* self;
  vec3 initial_origin = {0.3f, 1.1f, 0.0f};

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  glm_vec3_copy(initial_origin, self->origin);
  self->angle.polar = GLM_PI / 3;
  self->angle.azimuthal = GLM_PI * 1.3;

  wl_signal_init(&self->events.destroy);

  return self;

err:
  return NULL;
}

void
zn_ray_destroy(struct zn_ray* self)
{
  wl_signal_emit(&self->events.destroy, NULL);
  free(self);
}
