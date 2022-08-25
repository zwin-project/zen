#include "zen/xdg-toplevel-view.h"

#include "zen-common.h"
#include "zen/scene/scene.h"
#include "zen/scene/screen-layout.h"
#include "zen/scene/view.h"

static void zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self);

static void
zn_xdg_toplevel_view_map(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, map_listener);
  struct zn_scene* scene = self->server->scene;
  UNUSED(data);

  if (!zn_assert(!zn_view_is_mapped(&self->base), "Tried to map a mapped view"))
    return;

  zn_view_map_to_scene(&self->base, scene);
}

static void
zn_xdg_toplevel_view_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xdg_toplevel_view* self =
      zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  if (!zn_assert(
          zn_view_is_mapped(&self->base), "Tried to unmap an unmapped view"))
    return;

  zn_view_unmap(&self->base);
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

static void
zn_xdg_toplevel_view_impl_set_activated(struct zn_view* view, bool active)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  wlr_xdg_toplevel_set_activated(self->wlr_xdg_toplevel->base, active);
}

static struct wlr_surface*
zn_xdg_toplevel_view_impl_get_wlr_surface(struct zn_view* view)
{
  struct zn_xdg_toplevel_view* self = zn_container_of(view, self, base);
  return self->wlr_xdg_toplevel->base->surface;
}

static const struct zn_view_impl zn_xdg_toplevel_view_impl = {
    .get_wlr_surface = zn_xdg_toplevel_view_impl_get_wlr_surface,
    .set_activated = zn_xdg_toplevel_view_impl_set_activated,
};

struct zn_xdg_toplevel_view*
zn_xdg_toplevel_view_create(
    struct wlr_xdg_toplevel* wlr_xdg_toplevel, struct zn_server* server)
{
  struct zn_xdg_toplevel_view* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    wl_resource_post_no_memory(wlr_xdg_toplevel->resource);
    goto err;
  }

  self->server = server;

  zn_view_init(&self->base, ZN_VIEW_XDG_TOPLEVEL, &zn_xdg_toplevel_view_impl);

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

  return self;

err:
  return NULL;
}

static void
zn_xdg_toplevel_view_destroy(struct zn_xdg_toplevel_view* self)
{
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  zn_view_fini(&self->base);
  free(self);
}
