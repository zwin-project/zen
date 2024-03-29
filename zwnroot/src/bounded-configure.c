#include "bounded-configure.h"

#include <cglm/vec3.h>
#include <zen-common.h>

struct zwnr_bounded_configure *
zwnr_bounded_configure_create(struct wl_display *display, vec3 half_size)
{
  struct zwnr_bounded_configure *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  glm_vec3_copy(half_size, self->half_size);
  wl_list_init(&self->link);
  self->serial = wl_display_next_serial(display);

  return self;

err:
  return NULL;
}

void
zwnr_bounded_configure_destroy(struct zwnr_bounded_configure *self)
{
  wl_list_remove(&self->link);
  free(self);
}
