#include "zen/screen/xwayland.h"

#include <zen-common.h>

#include "zen/screen/cursor-grab/move.h"
#include "zen/screen/cursor-grab/resize.h"
#include "zen/server.h"
#include "zen/view.h"

static void zn_xwayland_view_destroy(struct zn_xwayland_view *self);

static struct wlr_surface *
zn_xwayland_view_impl_get_wlr_surface_at(struct zn_view *view, double view_sx,
    double view_sy, double *surface_x, double *surface_y)
{
  struct zn_xwayland_view *self = view->user_data;

  return wlr_surface_surface_at(
      self->xwayland_surface->surface, view_sx, view_sy, surface_x, surface_y);
}

static void
zn_xwayland_view_impl_get_window_geom(struct zn_view *view, struct wlr_box *box)
{
  struct zn_xwayland_view *self = view->user_data;
  box->x = 0;
  box->y = 0;
  box->width = self->xwayland_surface->width;
  box->height = self->xwayland_surface->height;
}

static uint32_t
zn_xwayland_view_impl_get_current_configure_serial(struct zn_view *view)
{
  UNUSED(view);
  return 0;
}

static uint32_t
zn_xwayland_view_impl_set_size(
    struct zn_view *view, double width, double height)
{
  struct zn_xwayland_view *self = view->user_data;

  wlr_xwayland_surface_configure(self->xwayland_surface,
      self->xwayland_surface->x, self->xwayland_surface->y, width, height);

  return 0;
}

static void
zn_xwayland_view_impl_set_position(struct zn_view *view, double x, double y)
{
  struct zn_xwayland_view *self = view->user_data;

  wlr_xwayland_surface_configure(self->xwayland_surface, x, y,
      self->xwayland_surface->width, self->xwayland_surface->height);
}

static uint32_t
zn_xwayland_view_impl_set_maximized(struct zn_view *view, bool maximized)
{
  struct zn_xwayland_view *self = view->user_data;

  wlr_xwayland_surface_set_maximized(self->xwayland_surface, maximized);

  return 0;
}

static void
zn_xwayland_view_impl_set_activated(struct zn_view *view, bool activated)
{
  struct zn_xwayland_view *self = view->user_data;

  wlr_xwayland_surface_activate(self->xwayland_surface, activated);
}

static void
zn_xwayland_view_impl_restack(struct zn_view *view, enum xcb_stack_mode_t mode)
{
  struct zn_xwayland_view *self = view->user_data;
  wlr_xwayland_surface_restack(self->xwayland_surface, NULL, mode);
}

static const struct zn_view_interface zn_xwayland_view_impl = {
    .get_wlr_surface_at = zn_xwayland_view_impl_get_wlr_surface_at,
    .get_window_geom = zn_xwayland_view_impl_get_window_geom,
    .get_current_configure_serial =
        zn_xwayland_view_impl_get_current_configure_serial,
    .set_size = zn_xwayland_view_impl_set_size,
    .set_maximized = zn_xwayland_view_impl_set_maximized,
    .set_position = zn_xwayland_view_impl_set_position,
    .set_activated = zn_xwayland_view_impl_set_activated,
    .restack = zn_xwayland_view_impl_restack,
};

static void
zn_xwayland_view_handle_move(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xwayland_view *self =
      zn_container_of(listener, self, move_listener);
  struct zn_server *server = zn_server_get_singleton();

  zn_move_cursor_grab_start(server->scene->cursor, self->view);
}

static void
zn_xwayland_view_handle_resize(struct wl_listener *listener, void *data)
{
  struct zn_xwayland_view *self =
      zn_container_of(listener, self, resize_listener);
  struct wlr_xwayland_resize_event *event = data;
  struct zn_server *server = zn_server_get_singleton();

  zn_resize_cursor_grab_start(server->scene->cursor, self->view, event->edges);
}

static void
zn_xwayland_view_handle_maximize(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xwayland_view *self =
      zn_container_of(listener, self, maximize_listener);
  if (!self->view) {
    return;
  }
  zn_view_set_maximized(self->view, !self->view->maximize_status.maximized);
}

static void
zn_xwayland_view_handle_map(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_xwayland_view *self = zn_container_of(listener, self, map_listener);

  if (!zn_assert(self->view == NULL, "xwayland has already mapped")) {
    return;
  }

  self->view = zn_view_create(
      self->xwayland_surface->surface, &zn_xwayland_view_impl, self);
  zn_scene_new_view(server->scene, self->view);
}

static void
zn_xwayland_view_handle_unmap(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_xwayland_view *self =
      zn_container_of(listener, self, unmap_listener);

  if (!zn_assert(self->view, "xwayland has not mapped yet")) {
    return;
  }

  zn_view_destroy(self->view);
  self->view = NULL;
}

static void
zn_xwayland_view_handle_xwayland_surface_destroy(
    struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_xwayland_view *self =
      zn_container_of(listener, self, wlr_xwayland_surface_destroy_listener);

  zn_xwayland_view_destroy(self);
}

struct zn_xwayland_view *
zn_xwayland_view_create(struct wlr_xwayland_surface *xwayland_surface)
{
  struct zn_xwayland_view *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->xwayland_surface = xwayland_surface;
  self->view = NULL;

  self->wlr_xwayland_surface_destroy_listener.notify =
      zn_xwayland_view_handle_xwayland_surface_destroy;
  wl_signal_add(&xwayland_surface->events.destroy,
      &self->wlr_xwayland_surface_destroy_listener);

  self->map_listener.notify = zn_xwayland_view_handle_map;
  wl_signal_add(&xwayland_surface->events.map, &self->map_listener);

  self->unmap_listener.notify = zn_xwayland_view_handle_unmap;
  wl_signal_add(&xwayland_surface->events.unmap, &self->unmap_listener);

  self->move_listener.notify = zn_xwayland_view_handle_move;
  wl_signal_add(&xwayland_surface->events.request_move, &self->move_listener);

  self->resize_listener.notify = zn_xwayland_view_handle_resize;
  wl_signal_add(
      &xwayland_surface->events.request_resize, &self->resize_listener);

  self->maximize_listener.notify = zn_xwayland_view_handle_maximize;
  wl_signal_add(
      &xwayland_surface->events.request_maximize, &self->maximize_listener);

  return self;

err:
  return NULL;
}

static void
zn_xwayland_view_destroy(struct zn_xwayland_view *self)
{
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->resize_listener.link);
  wl_list_remove(&self->maximize_listener.link);
  wl_list_remove(&self->wlr_xwayland_surface_destroy_listener.link);
  if (self->view) zn_view_destroy(self->view);
  free(self);
}
