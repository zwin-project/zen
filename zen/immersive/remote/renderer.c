#include "zen/immersive/remote/renderer.h"

#include "zen-common.h"
#include "zen/immersive/remote/object/board.h"

void
zn_remote_immersive_renderer_activate(struct zn_remote_immersive_renderer* self)
{
  zn_remote_scene_start_sync(self->remote_scene);
}

void
zn_remote_immersive_renderer_deactivate(
    struct zn_remote_immersive_renderer* self)
{
  zn_remote_scene_stop_sync(self->remote_scene);
}

struct zn_immersive_renderer*
zn_immersive_renderer_create(struct zn_scene* scene)
{
  struct zn_remote_immersive_renderer* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->remote_scene = zn_remote_scene_create(scene);
  if (self->remote_scene == NULL) {
    zn_error("Failed to create a remote scene");
    goto err_free;
  }

  return &self->base;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_immersive_renderer_destroy(struct zn_immersive_renderer* parent)
{
  struct zn_remote_immersive_renderer* self =
      zn_container_of(parent, self, base);

  zn_remote_scene_destroy(self->remote_scene);
  free(self);
}
