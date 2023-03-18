#include "layer-surface.h"

#include <cglm/vec2.h>
#include <zen/snode.h>

#include "output.h"
#include "screen-backend.h"
#include "surface-snode.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/screen.h"

static void zn_layer_surface_destroy(struct zn_layer_surface *self);

static void
zn_layer_surface_handle_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_layer_surface_destroy(self);
}

static void
zn_layer_surface_handle_surface_map(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_map_listener);

  wl_signal_add(&self->wlr_layer_surface->surface->events.commit,
      &self->surface_commit_listener);

  zn_surface_snode_damage_whole(self->surface_snode);
}

static void
zn_layer_surface_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);

  zn_surface_snode_damage_whole(self->surface_snode);
}

static void
zn_layer_surface_handle_surface_commit(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_commit_listener);

  bool layer_changed = self->layer != self->wlr_layer_surface->current.layer;

  if (layer_changed) {
    zn_screen_backend_add_layer_surface(self->output->screen_backend, self,
        self->wlr_layer_surface->current.layer);
    zn_surface_snode_damage_whole(self->surface_snode);
  } else {
    zn_surface_snode_commit_damage(self->surface_snode);
  }
}

static void
zn_layer_surface_handle_output_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, output_destroy_listener);

  // This triggers surface_destroy_listener and then destroys self.
  wlr_layer_surface_v1_destroy(self->wlr_layer_surface);
}

const struct zn_snode_interface zn_layer_surface_snode_implementation = {
    .get_texture = zn_snode_noop_get_texture,
    .frame = zn_snode_noop_frame,
    .accepts_input = zn_snode_noop_accepts_input,
    .pointer_button = zn_snode_noop_pointer_button,
    .pointer_enter = zn_snode_noop_pointer_enter,
    .pointer_motion = zn_snode_noop_pointer_motion,
    .pointer_leave = zn_snode_noop_pointer_leave,
    .pointer_axis = zn_snode_noop_pointer_axis,
    .pointer_frame = zn_snode_noop_pointer_frame,
    .on_focus = zn_snode_noop_on_focus,
};

struct zn_layer_surface *
zn_layer_surface_create(
    struct wlr_layer_surface_v1 *wlr_layer_surface, struct zn_output *output)
{
  struct zn_layer_surface *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->output = output;
  self->wlr_layer_surface = wlr_layer_surface;
  self->layer = wlr_layer_surface->current.layer;

  self->snode = zn_snode_create(self, &zn_layer_surface_snode_implementation);
  if (self->snode == NULL) {
    zn_error("Failed to create a zn_snode");
    goto err_free;
  }

  self->surface_snode = zn_surface_snode_create(wlr_layer_surface->surface);
  if (self->surface_snode == NULL) {
    zn_error("Failed to create a surface snode");
    goto err_snode;
  }

  zn_snode_set_position(self->surface_snode->snode, self->snode, GLM_VEC2_ZERO);

  self->surface_destroy_listener.notify =
      zn_layer_surface_handle_surface_destroy;
  wl_signal_add(
      &wlr_layer_surface->events.destroy, &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_layer_surface_handle_surface_map;
  wl_signal_add(&wlr_layer_surface->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify = zn_layer_surface_handle_surface_unmap;
  wl_signal_add(
      &wlr_layer_surface->events.unmap, &self->surface_unmap_listener);

  self->surface_commit_listener.notify = zn_layer_surface_handle_surface_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->output_destroy_listener.notify = zn_layer_surface_handle_output_destroy;
  wl_signal_add(&output->events.destroy, &self->output_destroy_listener);

  return self;

err_snode:
  zn_snode_destroy(self->snode);

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_layer_surface_destroy(struct zn_layer_surface *self)
{
  wl_list_remove(&self->output_destroy_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_snode_destroy(self->snode);
  free(self);
}
