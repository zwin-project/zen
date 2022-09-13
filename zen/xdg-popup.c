#include "zen/xdg-popup.h"

#include "zen-common.h"
#include "zen/scene/view-child.h"

static void zn_xdg_popup_destroy(struct zn_xdg_popup* self);

static void
zn_xdg_popup_map(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self = zn_container_of(listener, self, map_listener);
  UNUSED(data);

  // zn_view_child_is_mapped check

  zn_view_child_damage(&self->base);

  wl_signal_add(&self->wlr_xdg_popup->base->surface->events.commit,
      &self->wlr_surface_commit_listener);
}

static void
zn_xdg_popup_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self = zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  // zn_view_child_is_mapped check

  zn_view_child_damage(&self->base);

  wl_list_remove(&self->wlr_surface_commit_listener.link);
  wl_list_init(&self->wlr_surface_commit_listener.link);
}

static void
zn_xdg_popup_wlr_surface_commit_handler(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self =
      zn_container_of(listener, self, wlr_surface_commit_listener);
  UNUSED(data);

  zn_view_child_damage(&self->base);
}

static void
zn_xdg_popup_wlr_xdg_surface_destroy_handler(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_popup* self =
      zn_container_of(listener, self, wlr_xdg_surface_destroy_listener);
  UNUSED(data);

  zn_xdg_popup_destroy(self);
}

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

  // self->server = server;

  zn_view_child_init(&self->base, view);

  self->wlr_xdg_popup = wlr_xdg_popup;

  self->map_listener.notify = zn_xdg_popup_map;
  wl_signal_add(&self->wlr_xdg_popup->base->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xdg_popup_unmap;
  wl_signal_add(
      &self->wlr_xdg_popup->base->events.unmap, &self->unmap_listener);

  self->wlr_xdg_surface_destroy_listener.notify =
      zn_xdg_popup_wlr_xdg_surface_destroy_handler;
  wl_signal_add(&wlr_xdg_popup->base->events.destroy,
      &self->wlr_xdg_surface_destroy_listener);

  self->wlr_surface_commit_listener.notify =
      zn_xdg_popup_wlr_surface_commit_handler;
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
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  free(self);
}
