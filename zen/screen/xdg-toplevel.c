#include "zen/screen/xdg-toplevel.h"

#include <zen-common.h>

#include "zen/server.h"
#include "zen/view.h"

static void zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self);

static struct wlr_surface *
zn_xdg_toplevel_view_impl_get_wlr_surface_at(struct zn_view *view,
    double view_sx, double view_sy, double *surface_x, double *surface_y)
{
  struct wlr_surface *s =
      wlr_xdg_surface_surface_at(view->xdg_toplevel->wlr_xdg_toplevel->base,
          view_sx, view_sy, surface_x, surface_y);
  return s;
}

static const struct zn_view_impl zn_xdg_toplevel_view_impl = {
    .get_wlr_surface_at = zn_xdg_toplevel_view_impl_get_wlr_surface_at,
};

static void
zn_xdg_toplevel_handle_map(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_xdg_toplevel *self = zn_container_of(listener, self, map_listener);

  if (!zn_assert(self->view == NULL, "xdg toplevel has already mapped")) {
    return;
  }

  self->view = zn_view_create(self->wlr_xdg_toplevel->base->surface);
  self->view->impl = &zn_xdg_toplevel_view_impl;
  self->view->xdg_toplevel = self;
  zn_scene_new_view(server->scene, self->view);
}

static void
zn_xdg_toplevel_handle_unmap(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, unmap_listener);

  if (!zn_assert(self->view, "xdg toplevel has not mapped yet")) {
    return;
  }

  zn_view_destroy(self->view);
  self->view = NULL;
}

static void
zn_xdg_toplevel_handle_xdg_surface_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);

  zn_xdg_toplevel_destroy(self);
}

struct zn_xdg_toplevel *
zn_xdg_toplevel_create(struct wlr_xdg_toplevel *toplevel)
{
  struct zn_xdg_toplevel *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_xdg_toplevel = toplevel;
  self->view = NULL;

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_toplevel_handle_xdg_surface_destroy;
  wl_signal_add(
      &toplevel->base->events.destroy, &self->wlr_xdg_surface_destroy_listener);

  self->map_listener.notify = zn_xdg_toplevel_handle_map;
  wl_signal_add(&toplevel->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_toplevel_handle_unmap;
  wl_signal_add(&toplevel->base->events.unmap, &self->unmap_listener);

  return self;

err:
  return NULL;
}

static void
zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self)
{
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  if (self->view) zn_view_destroy(self->view);
  free(self);
}
