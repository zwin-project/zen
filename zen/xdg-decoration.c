#include "zen/xdg-decoration.h"

#include "zen-common.h"
#include "zen/decoration-manager.h"
#include "zen/xdg-toplevel-view.h"

static void
zn_xdg_decoration_set_view_decoration(struct zn_xdg_decoration* self)
{
  enum wlr_xdg_toplevel_decoration_v1_mode mode =
      WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

  if (self->wlr_decoration->requested_mode) {
    mode = self->wlr_decoration->requested_mode;
  }

  self->view->requested_client_decoration =
      mode == WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

  wlr_xdg_toplevel_decoration_v1_set_mode(self->wlr_decoration, mode);
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
  zn_xdg_decoration_destroy(self);
}

static void
zn_xdg_decoration_handle_decoration_destroy(
    struct wl_listener* listener, void* data)
{
  struct zn_xdg_decoration* self =
      zn_container_of(listener, self, decoration_destroy_listener);
  UNUSED(data);
  zn_xdg_decoration_destroy(self);
}

struct zn_xdg_decoration*
zn_xdg_decoration_create(struct wlr_xdg_toplevel_decoration_v1* decoration)
{
  if (!zn_assert(decoration->surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL,
          "decoration's surface role isn't toplevel")) {
    goto err;
  }

  struct zn_xdg_decoration* self;
  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->view = decoration->surface->data;
  self->wlr_decoration = decoration;

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
zn_xdg_decoration_destroy(struct zn_xdg_decoration* self)
{
  wl_list_remove(&self->request_mode_listener.link);
  wl_list_remove(&self->view_destroy_listener.link);
  wl_list_remove(&self->decoration_destroy_listener.link);
  free(self);
}
