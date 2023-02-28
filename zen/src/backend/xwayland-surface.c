#include "xwayland-surface.h"

#include "backend.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/server.h"
#include "zen/view.h"

static void zn_xwayland_surface_destroy(struct zn_xwayland_surface *self);

static struct wlr_texture *
zn_xwayland_surface_handle_get_texture(void *impl_data)
{
  struct zn_xwayland_surface *self = impl_data;
  return wlr_surface_get_texture(self->wlr_xsurface->surface);
}

static void
zn_xwayland_surface_handle_frame(void *impl_data, const struct timespec *when)
{
  struct zn_xwayland_surface *self = impl_data;

  wlr_surface_send_frame_done(self->wlr_xsurface->surface, when);
}

static const struct zn_view_interface view_implementation = {
    .get_texture = zn_xwayland_surface_handle_get_texture,
    .frame = zn_xwayland_surface_handle_frame,
};

static void
zn_xwayland_surface_handle_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_xwayland_surface_destroy(self);
}

static void
zn_xwayland_surface_handle_surface_map(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_map_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);

  zn_default_backend_notify_view_mapped(backend, self->view);
}

static void
zn_xwayland_surface_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  zn_view_notify_unmap(self->view);
}

struct zn_xwayland_surface *
zn_xwayland_surface_create(struct wlr_xwayland_surface *wlr_xsurface)
{
  struct zn_xwayland_surface *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_xsurface = wlr_xsurface;

  self->view = zn_view_create(self, &view_implementation);
  if (self->view == NULL) {
    zn_error("Failed to create a zn_view");
    goto err_free;
  }

  self->surface_destroy_listener.notify =
      zn_xwayland_surface_handle_surface_destroy;
  wl_signal_add(&wlr_xsurface->events.destroy, &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_xwayland_surface_handle_surface_map;
  wl_signal_add(&wlr_xsurface->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify =
      zn_xwayland_surface_handle_surface_unmap;
  wl_signal_add(&wlr_xsurface->events.unmap, &self->surface_unmap_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_xwayland_surface_destroy(struct zn_xwayland_surface *self)
{
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_view_destroy(self->view);
  free(self);
}
