#include "layer-surface.h"

#include "zen-common/log.h"
#include "zen-common/util.h"

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
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_map_listener);

  wl_signal_add(&self->wlr_layer_surface->surface->events.commit,
      &self->surface_commit_listener);
}

static void
zn_layer_surface_handle_surface_unmap(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_layer_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);
}

static void
zn_layer_surface_handle_surface_commit(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{}

struct zn_layer_surface *
zn_layer_surface_create(struct wlr_layer_surface_v1 *wlr_layer_surface)
{
  struct zn_layer_surface *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_layer_surface = wlr_layer_surface;

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

  return self;

err:
  return NULL;
}

static void
zn_layer_surface_destroy(struct zn_layer_surface *self)
{
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  free(self);
}
