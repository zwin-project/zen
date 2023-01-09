#include "zen/screen/xdg-toplevel.h"

#include <zen-common.h>

#include "zen/screen/cursor-grab/move.h"
#include "zen/screen/cursor-grab/resize.h"
#include "zen/server.h"
#include "zen/view.h"

static void zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self);

static struct wlr_surface *
zn_xdg_toplevel_view_impl_get_wlr_surface_at(struct zn_view *view,
    double view_sx, double view_sy, double *surface_x, double *surface_y)
{
  struct zn_xdg_toplevel *self = view->user_data;

  return wlr_xdg_surface_surface_at(
      self->wlr_xdg_toplevel->base, view_sx, view_sy, surface_x, surface_y);
}

static void
zn_xdg_toplevel_view_impl_get_window_geom(
    struct zn_view *view, struct wlr_box *box)
{
  struct zn_xdg_toplevel *self = view->user_data;
  wlr_xdg_surface_get_geometry(self->wlr_xdg_toplevel->base, box);
}

static uint32_t
zn_xdg_toplevel_view_impl_get_current_configure_serial(struct zn_view *view)
{
  struct zn_xdg_toplevel *self = view->user_data;
  return self->wlr_xdg_toplevel->base->current.configure_serial;
}

static uint32_t
zn_xdg_toplevel_view_impl_set_size(
    struct zn_view *view, double width, double height)
{
  struct zn_xdg_toplevel *self = view->user_data;

  return wlr_xdg_toplevel_set_size(self->wlr_xdg_toplevel->base, width, height);
}

static uint32_t
zn_xdg_toplevel_view_impl_set_maximized(struct zn_view *view, bool maximized)
{
  struct zn_xdg_toplevel *self = view->user_data;

  return wlr_xdg_toplevel_set_maximized(
      self->wlr_xdg_toplevel->base, maximized);
}

static void
zn_xdg_toplevel_view_impl_set_activated(struct zn_view *view, bool activated)
{
  struct zn_xdg_toplevel *self = view->user_data;

  wlr_xdg_toplevel_set_activated(self->wlr_xdg_toplevel->base, activated);
}

static uint32_t
zn_xdg_toplevel_view_impl_schedule_configure(struct zn_view *view)
{
  struct zn_xdg_toplevel *self = view->user_data;

  return wlr_xdg_surface_schedule_configure(self->wlr_xdg_toplevel->base);
}

static const struct zn_view_interface zn_xdg_toplevel_view_impl = {
    .get_wlr_surface_at = zn_xdg_toplevel_view_impl_get_wlr_surface_at,
    .get_window_geom = zn_xdg_toplevel_view_impl_get_window_geom,
    .get_current_configure_serial =
        zn_xdg_toplevel_view_impl_get_current_configure_serial,
    .set_size = zn_xdg_toplevel_view_impl_set_size,
    .set_maximized = zn_xdg_toplevel_view_impl_set_maximized,
    .set_activated = zn_xdg_toplevel_view_impl_set_activated,
    .schedule_configure = zn_xdg_toplevel_view_impl_schedule_configure,
};

static void
zn_xdg_toplevel_view_handle_move(struct wl_listener *listener, void *data)
{
  // FIXME: pointer/button/serial validation
  UNUSED(data);
  struct zn_xdg_toplevel *self = zn_container_of(listener, self, move_listener);
  struct zn_server *server = zn_server_get_singleton();

  zn_move_cursor_grab_start(server->scene->cursor, self->view);
}

static void
zn_xdg_toplevel_view_handle_resize(struct wl_listener *listener, void *data)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, resize_listener);
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct zn_server *server = zn_server_get_singleton();

  zn_resize_cursor_grab_start(server->scene->cursor, self->view, event->edges);
}

static void
zn_xdg_toplevel_view_handle_maximize(struct wl_listener *listener, void *data)
{
  UNUSED(data);
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, maximize_listener);

  if (self->view) {
    zn_view_set_maximized(
        self->view, self->wlr_xdg_toplevel->requested.maximized);
  }
}

static void
zn_xdg_toplevel_handle_map(struct wl_listener *listener, void *data)
{
  UNUSED(data);

  struct zn_server *server = zn_server_get_singleton();
  struct zn_xdg_toplevel *self = zn_container_of(listener, self, map_listener);

  if (!zn_assert(self->view == NULL, "xdg toplevel has already mapped")) {
    return;
  }

  self->view = zn_view_create(
      self->wlr_xdg_toplevel->base->surface, &zn_xdg_toplevel_view_impl, self);
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

  self->move_listener.notify = zn_xdg_toplevel_view_handle_move;
  wl_signal_add(&toplevel->events.request_move, &self->move_listener);

  self->resize_listener.notify = zn_xdg_toplevel_view_handle_resize;
  wl_signal_add(&toplevel->events.request_resize, &self->resize_listener);

  self->maximize_listener.notify = zn_xdg_toplevel_view_handle_maximize;
  wl_signal_add(&toplevel->events.request_maximize, &self->maximize_listener);

  return self;

err:
  return NULL;
}

static void
zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self)
{
  wl_list_remove(&self->unmap_listener.link);
  wl_list_remove(&self->map_listener.link);
  wl_list_remove(&self->move_listener.link);
  wl_list_remove(&self->resize_listener.link);
  wl_list_remove(&self->maximize_listener.link);
  wl_list_remove(&self->wlr_xdg_surface_destroy_listener.link);
  if (self->view) zn_view_destroy(self->view);
  free(self);
}
