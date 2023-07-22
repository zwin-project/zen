#include "xdg-decoration.h"

#include "zen-common/log.h"
#include "zen-common/signal.h"
#include "zen-common/util.h"

static void zn_xdg_decoration_destroy(struct zn_xdg_decoration *self);

static void
zn_xdg_decoration_handle_wlr_decoration_destroy(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_decoration *self =
      zn_container_of(listener, self, wlr_decoration_destroy_listener);

  zn_xdg_decoration_destroy(self);
}

static void
zn_xdg_decoration_handle_request_mode(
    struct wl_listener *listener, void *data UNUSED)
{
  struct zn_xdg_decoration *self =
      zn_container_of(listener, self, wlr_decoration_request_mode_listener);

  // TODO(@Aki-7): make this configurable from the desktop component
  enum wlr_xdg_toplevel_decoration_v1_mode mode =
      WLR_XDG_TOPLEVEL_DECORATION_V1_MODE_CLIENT_SIDE;

  enum wlr_xdg_toplevel_decoration_v1_mode client_mode =
      self->wlr_decoration->requested_mode;

  if (client_mode) {
    mode = client_mode;
  }

  wlr_xdg_toplevel_decoration_v1_set_mode(self->wlr_decoration, mode);
}

struct zn_xdg_decoration *
zn_xdg_decoration_create(struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration)
{
  struct zn_xdg_decoration *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_decoration = wlr_decoration;
  wl_signal_init(&self->events.destroy);

  self->wlr_decoration_destroy_listener.notify =
      zn_xdg_decoration_handle_wlr_decoration_destroy;
  wl_signal_add(&self->wlr_decoration->events.destroy,
      &self->wlr_decoration_destroy_listener);

  self->wlr_decoration_request_mode_listener.notify =
      zn_xdg_decoration_handle_request_mode;
  wl_signal_add(&self->wlr_decoration->events.request_mode,
      &self->wlr_decoration_request_mode_listener);

  /// For some clients, the set_mode request has already been processed at this
  /// point.
  zn_xdg_decoration_handle_request_mode(
      &self->wlr_decoration_request_mode_listener, wlr_decoration);

  return self;

err:
  return NULL;
}

static void
zn_xdg_decoration_destroy(struct zn_xdg_decoration *self)
{
  zn_signal_emit_mutable(&self->events.destroy, NULL);

  wl_list_remove(&self->events.destroy.listener_list);
  wl_list_remove(&self->wlr_decoration_request_mode_listener.link);
  wl_list_remove(&self->wlr_decoration_destroy_listener.link);
  free(self);
}
