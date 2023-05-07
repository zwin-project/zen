#include "bounded-configure.h"

#include <cglm/vec3.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_bounded_configure *
zn_bounded_configure_create(uint32_t serial, vec3 half_size)
{
  struct zn_bounded_configure *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->serial = serial;
  glm_vec3_copy(half_size, self->half_size);
  wl_list_init(&self->link);

  return self;

err:
  return NULL;
}

void
zn_bounded_configure_destroy(struct zn_bounded_configure *self)
{
  wl_list_remove(&self->link);
  free(self);
}
