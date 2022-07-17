#include "output.h"

#include "scene.h"
#include "toplevel-view.h"
#include "zen-common.h"
#include "zen-scene.h"

void
zn_scene_output_for_each_surface(struct zn_scene_output* self,
    void (*iterator)(struct wlr_surface* surface, void* data), void* data)
{
  struct zn_scene_toplevel_view* toplevel;
  wl_list_for_each(toplevel, &self->toplevels, link)
  {
    iterator(toplevel->wlr_xdg_toplevel->base->surface, data);
    // TODO: iterate popups and other things
  }
}

struct zn_scene_output*
zn_scene_output_create(struct zn_scene* scene, struct wlr_output* wlr_output)
{
  struct zn_scene_output* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->wlr_output = wlr_output;

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
