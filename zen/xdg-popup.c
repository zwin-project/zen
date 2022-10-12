#include "zen/xdg-popup.h"

#include "zen-common.h"
#include "zen/scene/view-child.h"

static void zn_xdg_popup_destroy(struct zn_xdg_popup* self);

static void
zn_xdg_popup_handle_map(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self = zn_container_of(listener, self, map_listener);
  UNUSED(data);

  zn_view_child_map(&self->base);

  wl_signal_add(&self->wlr_xdg_popup->base->surface->events.commit,
      &self->wlr_surface_commit_listener);
}

static void
zn_xdg_popup_handle_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self = zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  zn_view_child_unmap(&self->base);

  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_init(&self->wlr_surface_commit_listener.link);
}

static void
zn_xdg_popup_handle_new_popup(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self =
      zn_container_of(listener, self, new_popup_listener);
  struct wlr_xdg_popup* wlr_xdg_popup = data;

  zn_xdg_popup_create(wlr_xdg_popup, self->base.view);
}

static void
zn_xdg_popup_handle_wlr_surface_commit(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self =
      zn_container_of(listener, self, wlr_surface_commit_listener);
  UNUSED(data);

  zn_view_child_damage(&self->base);
}

static struct wlr_surface*
zn_xdg_popup_view_child_impl_get_wlr_surface(struct zn_view_child* child)
{
  struct zn_xdg_popup* self = zn_container_of(child, self, base);
  return self->wlr_xdg_popup->base->surface;
}

static void
zn_xdg_popup_view_child_impl_get_view_coords(
    struct zn_view_child* child, int* sx, int* sy)
{
  struct zn_xdg_popup* self = zn_container_of(child, self, base);
  struct wlr_xdg_surface* surface = self->wlr_xdg_popup->base;

  wlr_xdg_popup_get_toplevel_coords(surface->popup,
      surface->popup->geometry.x - surface->current.geometry.x,
      surface->popup->geometry.y - surface->current.geometry.y, sx, sy);
}

static void
zn_xdg_popup_handle_wlr_xdg_surface_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);
  UNUSED(data);

  zn_xdg_popup_destroy(self);
}

static const struct zn_view_child_impl zn_xdg_popup_view_child_impl = {
    .get_wlr_surface = zn_xdg_popup_view_child_impl_get_wlr_surface,
    .get_view_coords = zn_xdg_popup_view_child_impl_get_view_coords,
};

struct zn_xdg_popup*
zn_xdg_popup_create(struct wlr_xdg_popup* wlr_xdg_popup, struct zn_view* view)
{
  struct zn_xdg_popup* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_resource_post_no_memory(wlr_xdg_popup->resource);
    goto err;
  }

  zn_view_child_init(&self->base, NULL, &zn_xdg_popup_view_child_impl, view,
      wlr_xdg_popup->base->surface);

  self->wlr_xdg_popup = wlr_xdg_popup;

  self->map_listener.notify = zn_xdg_popup_handle_map;
  wl_signal_add(&self->wlr_xdg_popup->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_popup_handle_unmap;
  wl_signal_add(
      &self->wlr_xdg_popup->base->events.unmap, &self->unmap_listener);

  self->new_popup_listener.notify = zn_xdg_popup_handle_new_popup;
  wl_signal_add(
      &self->wlr_xdg_popup->base->events.new_popup, &self->new_popup_listener);

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_popup_handle_wlr_xdg_surface_destroy;
  wl_signal_add(&wlr_xdg_popup->base->events.destroy,
      &self->wlr_xdg_surface_destroy_listener);

  self->wlr_surface_commit_listener.notify =
      zn_xdg_popup_handle_wlr_surface_commit;
  wl_list_init(&self->wlr_surface_commit_listener.link);

  return self;

err:
  return NULL;
}

static void
zn_xdg_popup_destroy(struct zn_xdg_popup* self)
{
  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  wl_list_remove(&self->new_popup_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  free(self);
}
