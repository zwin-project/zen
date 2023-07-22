#include "xdg-toplevel.h"

#include <cglm/vec2.h>

#include "default-backend.h"
#include "surface-snode.h"
#include "xdg-decoration.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"
#include "zen/snode.h"
#include "zen/view.h"

static void zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self);

static void
zn_xdg_toplevel_set_focus(void *impl_data, bool focused)
{
  struct zn_xdg_toplevel *self = impl_data;
  struct zn_server *server = zn_server_get_singleton();
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(server->seat->wlr_seat);

  if (keyboard) {
    wlr_seat_keyboard_enter(server->seat->wlr_seat,
        self->wlr_xdg_toplevel->base->surface, keyboard->keycodes,
        keyboard->num_keycodes, &keyboard->modifiers);
  } else {
    wlr_seat_keyboard_enter(server->seat->wlr_seat,
        self->wlr_xdg_toplevel->base->surface, NULL, 0, NULL);
  }

  wlr_xdg_toplevel_set_activated(self->wlr_xdg_toplevel->base, focused);
}

static void
zn_xdg_toplevel_configure_size(void *impl_data, vec2 size)
{
  struct zn_xdg_toplevel *self = impl_data;

  wlr_xdg_toplevel_set_size(
      self->wlr_xdg_toplevel->base, (uint32_t)size[0], (uint32_t)size[1]);
}

static void
zn_xdg_toplevel_close(void *impl_data)
{
  struct zn_xdg_toplevel *self = impl_data;

  wlr_xdg_toplevel_send_close(self->wlr_xdg_toplevel->base);
}

static const struct zn_view_interface view_implementation = {
    .set_focus = zn_xdg_toplevel_set_focus,
    .configure_size = zn_xdg_toplevel_configure_size,
    .close = zn_xdg_toplevel_close,
};

static void
zn_xdg_toplevel_update_view_size(struct zn_xdg_toplevel *self)
{
  struct wlr_box geometry;
  wlr_xdg_surface_get_geometry(self->wlr_xdg_toplevel->base, &geometry);
  vec2 new_size = {(float)geometry.width, (float)geometry.height};

  if (!glm_vec2_eqv(self->view->size, new_size)) {
    zn_view_notify_resized(self->view, new_size);
  }
}

static void
zn_xdg_toplevel_update_surface_position(struct zn_xdg_toplevel *self)
{
  struct wlr_box geometry;
  wlr_xdg_surface_get_geometry(self->wlr_xdg_toplevel->base, &geometry);

  if (self->surface_snode == NULL) {
    return;
  }

  struct zn_snode *snode = self->surface_snode->snode;
  vec2 position = {(float)-geometry.x, (float)-geometry.y};

  zn_snode_change_position(snode, position);
}

static void
zn_xdg_toplevel_update_view_decoration(
    struct zn_xdg_toplevel *self, bool use_pending_state)
{
  enum wlr_xdg_toplevel_decoration_v1_mode wlr_mode =
      WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE;

  if (self->decoration) {
    wlr_mode = use_pending_state
                   ? self->decoration->wlr_decoration->pending.mode
                   : self->decoration->wlr_decoration->current.mode;
  }

  enum zn_view_decoration_mode mode = ZN_VIEW_DECORATION_MODE_CLIENT_SIDE;

  switch (wlr_mode) {
    case WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE:
      mode = ZN_VIEW_DECORATION_MODE_CLIENT_SIDE;
      break;
    case WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE:
      mode = ZN_VIEW_DECORATION_MODE_SERVER_SIDE;
      break;
    case WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_NONE:
      mode = ZN_VIEW_DECORATION_MODE_CLIENT_SIDE;
      break;
  }

  if (self->view->decoration_mode != mode) {
    zn_view_notify_decoration(self->view, mode);
  }
}

static void
zn_xdg_toplevel_handle_surface_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_destroy_listener);

  zn_xdg_toplevel_destroy(self);
}

static void
zn_xdg_toplevel_handle_surface_map(
    struct wl_listener *listener UNUSED, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_map_listener);
  struct zn_server *server = zn_server_get_singleton();
  struct zn_default_backend *backend = zn_default_backend_get(server->backend);

  wl_signal_add(&self->wlr_xdg_toplevel->base->surface->events.commit,
      &self->surface_commit_listener);

  self->surface_snode =
      zn_surface_snode_create(self->wlr_xdg_toplevel->base->surface);
  if (self->surface_snode == NULL) {
    zn_error("Failed to create a surface snode");
    wlr_xdg_toplevel_send_close(self->wlr_xdg_toplevel->base);
    return;
  }

  // TODO(@Aki-7): Respect window geometry
  zn_snode_set_position(
      self->surface_snode->snode, self->view->snode, GLM_VEC2_ZERO);

  zn_xdg_toplevel_update_surface_position(self);

  zn_xdg_toplevel_update_view_size(self);

  zn_xdg_toplevel_update_view_decoration(self, false);

  zn_default_backend_notify_view_mapped(backend, self->view);
}

static void
zn_xdg_toplevel_handle_surface_unmap(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_unmap_listener);

  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_init(&self->surface_commit_listener.link);

  zn_snode_set_position(self->surface_snode->snode, NULL, GLM_VEC2_ZERO);
  self->surface_snode = NULL;

  zn_view_notify_unmap(self->view);
}

static void
zn_xdg_toplevel_handle_surface_commit(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_commit_listener);

  if (!zn_assert_(self->surface_snode,
          "Commit signal must be handled only when the view is mapped")) {
    return;
  }

  zn_surface_snode_commit_damage(self->surface_snode);

  zn_xdg_toplevel_update_surface_position(self);

  zn_xdg_toplevel_update_view_size(self);

  // Use the pending state because the commit signal handler of the decoration
  // object can be called after this commit signal handler.
  zn_xdg_toplevel_update_view_decoration(self, true);
}

static void
zn_xdg_toplevel_handle_surface_move_request(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_move_request_listener);

  zn_view_notify_move_request(self->view);
}

static void
zn_xdg_toplevel_handle_surface_resize_request(
    struct wl_listener *listener, void *data)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, surface_resize_request_listener);

  struct wlr_xdg_toplevel_resize_event *wlr_event = data;
  struct zn_view_resize_event event;

  event.edges = wlr_event->edges;

  zn_view_notify_resize_request(self->view, &event);
}

static void
zn_xdg_toplevel_handle_decoration_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_toplevel *self =
      zn_container_of(listener, self, decoration_destroy_listener);

  zn_xdg_toplevel_set_decoration(self, NULL);
}

void
zn_xdg_toplevel_set_decoration(
    struct zn_xdg_toplevel *self, struct zn_xdg_decoration *decoration)
{
  if (self->decoration) {
    wl_list_remove(&self->decoration_destroy_listener.link);
    wl_list_init(&self->decoration_destroy_listener.link);
  }

  self->decoration = decoration;

  if (decoration) {
    wl_signal_add(
        &decoration->events.destroy, &self->decoration_destroy_listener);
  }
}

struct zn_xdg_toplevel *
zn_xdg_toplevel_from_wlr_xdg_surface(struct wlr_xdg_surface *surface)
{
  if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
    return surface->data;
  }

  return NULL;
}

struct zn_xdg_toplevel *
zn_xdg_toplevel_create(struct wlr_xdg_toplevel *wlr_xdg_toplevel)
{
  struct zn_xdg_toplevel *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_xdg_toplevel = wlr_xdg_toplevel;
  wlr_xdg_toplevel->base->data = self;

  self->view = zn_view_create(self, &view_implementation);
  if (self->view == NULL) {
    zn_error("Failed to create a zn_view");
    goto err_free;
  }

  self->decoration = NULL;
  self->surface_snode = NULL;

  self->surface_destroy_listener.notify =
      zn_xdg_toplevel_handle_surface_destroy;
  wl_signal_add(&self->wlr_xdg_toplevel->base->events.destroy,
      &self->surface_destroy_listener);

  self->surface_map_listener.notify = zn_xdg_toplevel_handle_surface_map;
  wl_signal_add(
      &self->wlr_xdg_toplevel->base->events.map, &self->surface_map_listener);

  self->surface_unmap_listener.notify = zn_xdg_toplevel_handle_surface_unmap;
  wl_signal_add(&self->wlr_xdg_toplevel->base->events.unmap,
      &self->surface_unmap_listener);

  self->surface_commit_listener.notify = zn_xdg_toplevel_handle_surface_commit;
  wl_list_init(&self->surface_commit_listener.link);

  self->surface_move_request_listener.notify =
      zn_xdg_toplevel_handle_surface_move_request;
  wl_signal_add(&self->wlr_xdg_toplevel->events.request_move,
      &self->surface_move_request_listener);

  self->surface_resize_request_listener.notify =
      zn_xdg_toplevel_handle_surface_resize_request;
  wl_signal_add(&self->wlr_xdg_toplevel->events.request_resize,
      &self->surface_resize_request_listener);

  self->decoration_destroy_listener.notify =
      zn_xdg_toplevel_handle_decoration_destroy;
  wl_list_init(&self->decoration_destroy_listener.link);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

static void
zn_xdg_toplevel_destroy(struct zn_xdg_toplevel *self)
{
  wl_list_remove(&self->decoration_destroy_listener.link);
  wl_list_remove(&self->surface_resize_request_listener.link);
  wl_list_remove(&self->surface_move_request_listener.link);
  wl_list_remove(&self->surface_commit_listener.link);
  wl_list_remove(&self->surface_unmap_listener.link);
  wl_list_remove(&self->surface_map_listener.link);
  wl_list_remove(&self->surface_destroy_listener.link);
  zn_view_destroy(self->view);
  free(self);
}
