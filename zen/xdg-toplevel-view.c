#include "zen/xdg-toplevel-view.h"

#include "zen-common.h"
#include "zen-scene.h"
#include "zen/view.h"

struct zn_xdg_toplevel_view {
  struct zn_view base;
  struct wlr_xdg_toplevel* wlr_xdg_toplevel;  // nonnull

  struct zn_server* server;
  struct zn_scene_toplevel_view* scene_view;  // nonnull if mapped

  struct wl_listener map_listener;
  struct wl_listener unmap_listener;
  struct wl_listener wlr_xdg_surface_destroy_listener;
};

static void zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self);

static void
zn_xdg_toplevel_view_map(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, map_listener);
  UNUSED(data);

  if (!zn_assert(self->scene_view == NULL, "Tried to map a mapped view"))
    return;

  self->scene_view = zn_scene_toplevel_view_create(
      zn_server_get_scene(self->server), self->wlr_xdg_toplevel, NULL);
}

static void
zn_xdg_toplevel_view_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  if (!zn_assert(self->scene_view, "Tried to unmap an unmapped view")) return;

  zn_scene_toplevel_view_destroy(self->scene_view);
  self->scene_view = NULL;
}

static void
zn_xdg_toplevel_view_wlr_xdg_surface_destroy_handler(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);
  UNUSED(data);

  zn_xdg_toplevel_view_destroy(self);
}

struct zn_xdg_toplevel_view*
zn_xdg_toplevel_view_create(
    struct wlr_xdg_toplevel* wlr_xdg_toplevel, struct zn_server* server)
{
  struct zn_xdg_toplevel_view* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("failed to allocate memory");
    wl_resource_post_no_memory(wlr_xdg_toplevel->resource);
    goto err;
  }

  self->server = server;

  zn_view_init(&self->base, ZN_VIEW_XDG_TOPLEVEL);

  self->wlr_xdg_toplevel = wlr_xdg_toplevel;

  self->map_listener.notify = zn_xdg_toplevel_view_map;
  wl_signal_add(&self->wlr_xdg_toplevel->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_toplevel_view_unmap;
  wl_signal_add(
      &self->wlr_xdg_toplevel->base->events.unmap, &self->unmap_listener);

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_toplevel_view_wlr_xdg_surface_destroy_handler;
  wl_signal_add(&wlr_xdg_toplevel->base->events.destroy,
      &self->wlr_xdg_surface_destroy_listener);

err:
  return NULL;
}

static void
zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self)
{
  if (self->scene_view) zn_scene_toplevel_view_destroy(self->scene_view);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  zn_view_fini(&self->base);
  free(self);
}
