#include "zen-common.h"
#include "zen-scene.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_scene {};
#pragma GCC diagnostic pop

struct zn_scene *
zn_scene_create()
{
  struct zn_scene *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  return self;

err:
  return NULL;
}

void
zn_scene_destroy(struct zn_scene *self)
{
  free(self);
}
