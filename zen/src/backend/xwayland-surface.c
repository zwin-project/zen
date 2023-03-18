#include "xwayland-surface.h"

#include <cglm/vec2.h>

#include "backend.h"
#include "surface-snode.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_xwayland_surface_destroy(struct zn_xwayland_surface *self);

static void
zn_xwayland_surface_set_focus(void *impl_data, bool focused)
{
  struct zn_xwayland_surface *self = impl_data;
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat->wlr_seat);

  if (keyboard) {
    wlr_seat_keyboard_enter(server->seat->wlr_seat, self->wlr_xsurface->surface,
        keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
  } else {
    wlr_seat_keyboard_enter(
        server->seat->wlr_seat, self->wlr_xsurface->surface, NULL, 0, NULL);
  }

  wlr_xwayland_surface_activate(self->wlr_xsurface, focused);

  wlr_xwayland_surface_restack(self->wlr_xsurface, NULL, XCB_STACK_MODE_ABOVE);
}

static void
zn_xwayland_surface_configure_size(void *impl_data, vec2 size)
{
  struct zn_xwayland_surface *self = impl_data;

  if (self->surface_snode == NULL ||
      self->surface_snode->snode->screen == NULL) {
    return;
  }

  struct wlr_fbox fbox;
  zn_snode_get_layout_fbox(self->surface_snode->snode, &fbox);

  wlr_xwayland_surface_configure(self->wlr_xsurface, (int16_t)fbox.x,
      (int16_t)fbox.y, (uint16_t)size[0], (uint16_t)size[1]);
}

static void
zn_xwayland_surface_close(void *impl_data)
{
  struct zn_xwayland_surface *self = impl_data;

  wlr_xwayland_surface_close(self->wlr_xsurface);
}

static const struct zn_view_interface view_implementation = {
    .set_focus = zn_xwayland_surface_set_focus,
    .configure_size = zn_xwayland_surface_configure_size,
    .close = zn_xwayland_surface_close,
};

static void
zn_xwayland_surface_update_view_size(struct zn_xwayland_surface *self)
{
  vec2 new_size = {(float)self->wlr_xsurface->surface->current.width,
      (float)self->wlr_xsurface->surface->current.height};

  if (self->view->size[0] != new_size[0] ||
      self->view->size[1] != new_size[1]) {
    zn_view_notify_resized(self->view, new_size);
  }
}

static void
zn_xwayland_surface_update_view_decoration(struct zn_xwayland_surface *self)
{
  enum zn_view_decoration_mode decoration_mode =
      self->wlr_xsurface->decorations != WLR_XWAYLAND_SURFACE_DECORATIONS_ALL
          ? ZN_VIEW_DECORATION_MODE_CLIENT_SIDE
          : ZN_VIEW_DECORATION_MODE_SERVER_SIDE;

  if (self->view->decoration_mode != decoration_mode) {
    zn_view_notify_decoration(self->view, decoration_mode);
  }
}

static void
zn_xwayland_surface_handle_snode_position_changed(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, snode_position_changed_listener);

  if (self->surface_snode == NULL ||
      self->surface_snode->snode->screen == NULL) {
    return;
  }

  struct wlr_fbox fbox;
  zn_snode_get_layout_fbox(self->surface_snode->snode, &fbox);

  wlr_xwayland_surface_configure(self->wlr_xsurface, (int16_t)fbox.x,
      (int16_t)fbox.y, self->wlr_xsurface->width, self->wlr_xsurface->height);
}

static void
zn_xwayland_surface_handle_move_request(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_move_request_listener);

  zn_view_notify_move_request(self->view);
}

static void
zn_xwayland_surface_handle_resize_request(
    struct wl_listener *listener, void *data)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_resize_request_listener);

  struct wlr_xwayland_resize_event *wlr_event = data;
  struct zn_view_resize_event event;

  event.edges = wlr_event->edges;

  zn_view_notify_resize_request(self->view, &event);
}

static void
zn_xwayland_surface_handle_set_decoration(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_set_decoration_listener);

  zn_xwayland_surface_update_view_decoration(self);
}

static void
zn_xwayland_surface_handle_configure(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_configure_listener);
  struct wlr_fbox fbox;
  struct wlr_xwayland_surface_configure_event *ev = data;

  if (!self->wlr_xsurface->mapped) {
    wlr_xwayland_surface_configure(
        self->wlr_xsurface, ev->x, ev->y, ev->width, ev->height);
    return;
  }

  zn_snode_get_layout_fbox(self->view->snode, &fbox);

  wlr_xwayland_surface_configure(self->wlr_xsurface, (int16_t)fbox.x,
      (int16_t)fbox.y, ev->width, ev->height);
}

static void
zn_xwayland_surface_handle_commit(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_commit_listener);

  if (!zn_assert(self->surface_snode,
          "Commit signal must be handled only when the view is mapped")) {
    return;
  }

  zn_surface_snode_commit_damage(self->surface_snode);

  zn_xwayland_surface_update_view_size(self);
}

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

  wl_signal_add(&self->wlr_xsurface->surface->events.commit,
      &self->surface_commit_listener);

  self->surface_snode = zn_surface_snode_create(self->wlr_xsurface->surface);
  if (self->surface_snode == NULL) {
    zn_error("Failed to create a surface snode");
    wlr_xwayland_surface_close(self->wlr_xsurface);
    return;
  }

  zn_snode_set_position(
      self->surface_snode->snode, self->view->snode, GLM_VEC2_ZERO);

  zn_xwayland_surface_update_view_size(self);

  zn_xwayland_surface_update_view_decoration(self);

  zn_default_backend_notify_view_mapped(backend, self->view);
}

static void
zn_xwayland_surface_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xwayland_surface *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);

  self->surface_snode = NULL;

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

  self->surface_snode = NULL;

  self->surface_destroy_listener.notify =
      zn_xwayland_surface_handle_surface_destroy;
  wl_signal_add(&wlr_xsurface->events.destroy, &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_xwayland_surface_handle_surface_map;
  wl_signal_add(&wlr_xsurface->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify =
      zn_xwayland_surface_handle_surface_unmap;
  wl_signal_add(&wlr_xsurface->events.unmap, &self->surface_unmap_listener);

  self->surface_configure_listener.notify =
      zn_xwayland_surface_handle_configure;
  wl_signal_add(&wlr_xsurface->events.request_configure,
      &self->surface_configure_listener);

  self->surface_move_request_listener.notify =
      zn_xwayland_surface_handle_move_request;
  wl_signal_add(
      &wlr_xsurface->events.request_move, &self->surface_move_request_listener);

  self->surface_resize_request_listener.notify =
      zn_xwayland_surface_handle_resize_request;
  wl_signal_add(&wlr_xsurface->events.request_resize,
      &self->surface_resize_request_listener);

  self->surface_set_decoration_listener.notify =
      zn_xwayland_surface_handle_set_decoration;
  wl_signal_add(&wlr_xsurface->events.set_decorations,
      &self->surface_set_decoration_listener);

  self->surface_commit_listener.notify = zn_xwayland_surface_handle_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->snode_position_changed_listener.notify =
      zn_xwayland_surface_handle_snode_position_changed;
  wl_signal_add(&self->view->snode->events.position_changed,
      &self->snode_position_changed_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_xwayland_surface_destroy(struct zn_xwayland_surface *self)
{
  wl_list_remove(&self->snode_position_changed_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_set_decoration_listener.link);
  wl_list_remove(&self->surface_resize_request_listener.link);
  wl_list_remove(&self->surface_move_request_listener.link);
  wl_list_remove(&self->surface_configure_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_view_destroy(self->view);
  free(self);
}
