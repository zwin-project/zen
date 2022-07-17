#include "output.h"

#include "scene.h"
#include "zen-common.h"
#include "zen-scene.h"

struct zn_scene_output*
zn_scene_output_create(struct zn_scene* scene)
{
  struct zn_scene_output* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  wl_list_init(&self->toplevels);
  wl_list_insert(&scene->output_list, &self->link);

  return self;

err:
  return NULL;
}

void
zn_scene_output_destroy(struct zn_scene_output* self)
{
  wl_list_remove(&self->toplevels);
  wl_list_remove(&self->link);
  free(self);
}
