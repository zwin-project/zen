#include "sphere.h"

#include <cglm/vec3.h>
#include <zen-common.h>

struct zgnr_sphere_region*
zgnr_sphere_region_create(vec3 center, float radius)
{
  struct zgnr_sphere_region* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  glm_vec3_copy(center, self->center);
  self->radius = radius;

  wl_list_init(&self->link);

  return self;

err:
  return NULL;
}

void
zgnr_sphere_region_destroy(struct zgnr_sphere_region* self)
{
  wl_list_remove(&self->link);
  free(self);
}
