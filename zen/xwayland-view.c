#include "zen/xwayland-view.h"

#include "zen-common.h"
#include "zen/scene/scene.h"
#include "zen/scene/view.h"

static void zn_xwayland_view_destroy(struct zn_xwayland_view* self);

static void
zn_xwayland_view_map(struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self = zn_container_of(listener, self, map_listener);
  struct zn_scene* scene = self->server->scene;
  struct zn_screen* screen;
  UNUSED(data);

  if (!zn_assert(!zn_view_is_mapped(&self->base), "Tried to map a mapped view"))
    return;

  if (wl_list_empty(&scene->screen_layout->screens)) {
    // TODO: handle this properly
    zn_warn("View is mapped but no screen found");
    return;
  }

  screen = zn_container_of(scene->screen_layout->screens.next, screen, link);

  zn_view_map_to_screen(&self->base, screen);
}

static void
zn_xwayland_view_unmap(struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, unmap_listener);
  UNUSED(data);

  if (!zn_assert(
          zn_view_is_mapped(&self->base), "Tried to unmap an unmapped view"))
    return;

  zn_view_unmap(&self->base);
}

static void
zn_xwayland_view_wlr_xwayland_surface_destroy_handler(
    struct wl_listener* listener, void* data)
{
  struct zn_xwayland_view* self =
      zn_container_of(listener, self, wlr_xwayland_surface_destroy_listener);
  UNUSED(data);

  zn_xwayland_view_destroy(self);
}

static struct wlr_surface*
zn_xwayland_view_impl_get_wlr_surface(struct zn_view* view)
{
  struct zn_xwayland_view* self = zn_container_of(view, self, base);
  return self->wlr_xwayland_surface->surface;
}

static const struct zn_view_impl zn_xwayland_view_impl = {
    .get_wlr_surface = zn_xwayland_view_impl_get_wlr_surface,
};

void
zn_xwayland_view_focus(struct zn_xwayland_view* self)
{
  wlr_xwayland_surface_activate(self->wlr_xwayland_surface, true);
  wlr_xwayland_surface_restack(
      self->wlr_xwayland_surface, NULL, XCB_STACK_MODE_ABOVE);
}

void
zn_xwayland_view_unfocus(struct zn_xwayland_view* self)
{
  wlr_xwayland_surface_activate(self->wlr_xwayland_surface, false);
}

struct zn_xwayland_view*
zn_xwayland_view_create(
    struct wlr_xwayland_surface* xwayland_surface, struct zn_server* server)
{
  struct zn_xwayland_view* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory.");
    wl_resource_post_no_memory(xwayland_surface->surface->resource);
    goto err;
  }

  self->server = server;

  zn_view_init(&self->base, ZN_VIEW_XWAYLAND, &zn_xwayland_view_impl);

  self->wlr_xwayland_surface = xwayland_surface;

  self->map_listener.notify = zn_xwayland_view_map;
  wl_signal_add(&self->wlr_xwayland_surface->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xwayland_view_unmap;
  wl_signal_add(
      &self->wlr_xwayland_surface->events.unmap, &self->unmap_listener);

  self->wlr_xwayland_surface_destroy_listener.notify =
      zn_xwayland_view_wlr_xwayland_surface_destroy_handler;
  wl_signal_add(&self->wlr_xwayland_surface->events.destroy,
      &self->wlr_xwayland_surface_destroy_listener);

  return self;

err:
  return NULL;
}

static void
zn_xwayland_view_destroy(struct zn_xwayland_view* self)
{
  wl_list_remove(&self->wlr_xwayland_surface_destroy_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->unmap_listener.link);
  zn_view_fini(&self->base);
  free(self);
}
