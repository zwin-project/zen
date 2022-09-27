#include "zen/xdg-decoration.h"

#include "zen-common.h"
#include "zen/decoration-manager.h"
#include "zen/xdg-toplevel-view.h"

static void
zn_xdg_decoration_set_view_decoration(struct zn_xdg_decoration* self)
{
  self->view->requested_client_decoration =
      self->wlr_decoration->requested_mode !=
      WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE;
}

static void
zn_xdg_decoration_handle_request_mode(struct wl_listener* listener, void* data)
{
  struct zn_xdg_decoration* self =
      zn_container_of(listener, self, request_mode_listener);
  UNUSED(data);
  zn_xdg_decoration_set_view_decoration(self);
}

static void
zn_xdg_decoration_handle_view_destroy(struct wl_listener* listener, void* data)
{
  struct zn_xdg_decoration* self =
      zn_container_of(listener, self, view_destroy_listener);
  UNUSED(data);
  zn_xdg_decoration_destory(self);
}

static void
zn_xdg_decoration_handle_decoration_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_decoration* self =
      zn_container_of(listener, self, decoration_destroy_listener);
  UNUSED(data);
  zn_xdg_decoration_destory(self);
}

struct zn_xdg_decoration*
zn_xdg_decoration_create(struct zn_decoration_manager* manager,
    struct wlr_xdg_toplevel_decoration_v1* decoration)
{
  struct zn_xdg_decoration* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->view = decoration->surface->data;
  self->wlr_decoration = decoration;

  wl_list_insert(&manager->xdg_decoration_list, &self->link);

  wlr_xdg_toplevel_decoration_v1_set_mode(
      self->wlr_decoration, WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE);

  zn_xdg_decoration_set_view_decoration(self);

  self->request_mode_listener.notify = zn_xdg_decoration_handle_request_mode;
  wl_signal_add(&decoration->events.request_mode, &self->request_mode_listener);

  self->view_destroy_listener.notify = zn_xdg_decoration_handle_view_destroy;
  wl_signal_add(&self->view->events.destroy, &self->view_destroy_listener);

  self->decoration_destroy_listener.notify =
      zn_xdg_decoration_handle_decoration_destroy;
  wl_signal_add(
      &decoration->events.destroy, &self->decoration_destroy_listener);

  return self;

err:
  return NULL;
}

void
zn_xdg_decoration_destory(struct zn_xdg_decoration* self)
{
  wl_list_remove(&self->link);
  wl_list_remove(&self->request_mode_listener.link);
  wl_list_remove(&self->view_destroy_listener.link);
  wl_list_remove(&self->decoration_destroy_listener.link);
  free(self);
}
