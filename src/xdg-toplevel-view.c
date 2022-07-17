#include "xdg-toplevel-view.h"

#include "view.h"
#include "zen-common.h"
#include "zen-scene.h"

struct zn_xdg_toplevel_view {
  struct zn_view base;
  struct wlr_xdg_toplevel* wlr_xdg_toplevel;  // nonnull

  struct zn_server* server;
  struct zn_scene_toplevel_view* scene_view;  // nonnull

  struct wl_listener wlr_xdg_surface_destroy_listener;
};

static void zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self);

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
  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_toplevel_view_wlr_xdg_surface_destroy_handler;
  wl_signal_add(&wlr_xdg_toplevel->base->events.destroy,
      &self->wlr_xdg_surface_destroy_listener);

  self->scene_view = zn_scene_toplevel_view_create(
      zn_server_get_scene(self->server), wlr_xdg_toplevel, NULL);

err:
  return NULL;
}

static void
zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self)
{
  zn_scene_toplevel_view_destroy(self->scene_view);
  zn_view_fini(&self->base);
  free(self);
}
