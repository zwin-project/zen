#include "zen/scene/scene.h"

#include "zen-common.h"

struct zn_scene*
zn_scene_create()
{
  struct zn_scene* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->screens);

  return self;

err:
  return NULL;
}

void
zn_scene_destroy(struct zn_scene* self)
{
  // ensure safety when zn_scene_output is destroyed
  wl_list_remove(&self->screens);

  free(self);
}
