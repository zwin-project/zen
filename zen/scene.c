#include "zen/scene.h"

#include <zen-common.h>

struct zn_scene *
zn_scene_create(void)
{
  struct zn_scene *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  wl_list_init(&self->board_list);

  return self;

err:
  return NULL;
}

void
zn_scene_destroy(struct zn_scene *self)
{
  // TODO: handle board_list
  free(self);
}
