#include "zen/screen/xdg-popup.h"

#include <zen-common.h>

#include "zen/screen/xdg-toplevel.h"
#include "zen/view-child.h"

void zn_xdg_popup_destroy(struct zn_xdg_popup *self);

static void
zn_xdg_popup_impl_get_toplevel_coords(struct zn_view_child *child,
    double child_sx, double child_sy, double *toplevel_sx, double *toplevel_sy)
{
  struct zn_xdg_popup *self = child->user_data;
  struct wlr_xdg_surface *surface = self->wlr_xdg_popup->base;

  int sx, sy;
  wlr_xdg_popup_get_toplevel_coords(surface->popup,
      surface->popup->geometry.x - surface->current.geometry.x,
      surface->popup->geometry.y - surface->current.geometry.y, &sx, &sy);

  *toplevel_sx = sx + child_sx;
  *toplevel_sy = sy + child_sy;
}

const struct zn_view_child_interface zn_xdg_popup_impl = {
    .get_toplevel_coords = zn_xdg_popup_impl_get_toplevel_coords,
};

static void
zn_xdg_popup_handle_map(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xdg_popup *self = zn_container_of(listener, self, map_listener);
  if (self->view_child) {
    zn_error("xdg toplevel has already mapped");
    return;
  }

  self->view_child = zn_view_child_create(self->wlr_xdg_popup->base->surface,
      self->toplevel->view, &zn_xdg_popup_impl, self);
}

static void
zn_xdg_popup_handle_unmap(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xdg_popup *self = zn_container_of(listener, self, unmap_listener);

  if (!self->view_child) {
    zn_error("xdg toplevel has not mapped yet");
    return;
  }

  zn_view_child_destroy(self->view_child);
  self->view_child = NULL;
}

static void
zn_xdg_popup_handle_new_popup(struct wl_listener *listener, void *data)
{
  struct zn_xdg_popup *self =
      zn_container_of(listener, self, new_popup_listener);
  struct wlr_xdg_popup *wlr_xdg_popup = data;
  zn_xdg_popup_create(wlr_xdg_popup, self->toplevel);
}

static void
zn_xdg_popup_handle_wlr_xdg_surface_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xdg_popup *self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);
  zn_xdg_popup_destroy(self);
}

struct zn_xdg_popup *
zn_xdg_popup_create(
    struct wlr_xdg_popup *popup, struct zn_xdg_toplevel *toplevel)
{
  struct zn_xdg_popup *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_xdg_popup = popup;
  self->toplevel = toplevel;

  self->map_listener.notify = zn_xdg_popup_handle_map;
  wl_signal_add(&popup->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_popup_handle_unmap;
  wl_signal_add(&popup->base->events.unmap, &self->unmap_listener);

  self->new_popup_listener.notify = zn_xdg_popup_handle_new_popup;
  wl_signal_add(&popup->base->events.new_popup, &self->new_popup_listener);

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_popup_handle_wlr_xdg_surface_destroy;
  wl_signal_add(
      &popup->base->events.destroy, &self->wlr_xdg_surface_destroy_listener);

  return self;

err:
  return NULL;
}

void
zn_xdg_popup_destroy(struct zn_xdg_popup *self)
{
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->new_popup_listener.link);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);

  if (self->view_child) {
    zn_view_child_destroy(self->view_child);
  }

  free(self);
}
