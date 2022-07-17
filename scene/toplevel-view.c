#include "toplevel-view.h"

#include "output.h"
#include "scene.h"
#include "zen-common.h"
#include "zen-scene.h"

struct zn_scene_toplevel_view*
zn_scene_toplevel_view_create(struct zn_scene* scene,
    struct wlr_xdg_toplevel* wlr_xdg_toplevel,
    struct zn_scene_output* output /* nullable */)
{
  struct zn_scene_toplevel_view* self;
  struct zn_scene_output* parent_output;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("No memory");
    wl_resource_post_no_memory(wlr_xdg_toplevel->resource);
    goto err;
  }

  self->wlr_xdg_toplevel = wlr_xdg_toplevel;

  if (output) {
    parent_output = output;
  } else if (wl_list_empty(&scene->output_list)) {
    goto err_free;
  } else {
    parent_output =
        zn_container_of(scene->output_list.next, parent_output, link);
  }

  wl_list_insert(&parent_output->toplevels, &self->link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_scene_toplevel_view_destroy(struct zn_scene_toplevel_view* self)
{
  wl_list_remove(&self->link);
  free(self);
}
