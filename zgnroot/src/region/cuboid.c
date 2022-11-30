#include "cuboid.h"

#include <cglm/quat.h>
#include <cglm/vec3.h>
#include <zen-common.h>

struct zgnr_cuboid_region *
zgnr_cuboid_region_create(vec3 half_size, vec3 center, versor quaternion)
{
  struct zgnr_cuboid_region *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  glm_vec3_copy(half_size, self->half_size);
  glm_vec3_copy(center, self->center);
  glm_vec4_copy(quaternion, self->quaternion);

  wl_list_init(&self->link);

  return self;

err:
  return NULL;
}

void
zgnr_cuboid_region_destroy(struct zgnr_cuboid_region *self)
{
  wl_list_remove(&self->link);
  free(self);
}
