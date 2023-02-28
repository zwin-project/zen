#include "backend/compositor.h"

#include "xwayland-surface.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

static void
zn_compositor_handle_new_xwayland_surface(
    struct wl_listener *listener UNUSED, void *data)
{
  struct wlr_xwayland_surface *wlr_xsurface = data;

  zn_xwayland_surface_create(wlr_xsurface);
}

struct zn_compositor *
zn_compositor_create(struct wl_display *display, struct wlr_renderer *renderer)
{
  char socket_name_candidate[16];
  const char *socket = NULL;
  const char *xdg = NULL;

  struct zn_compositor *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->wlr_compositor = wlr_compositor_create(display, renderer);
  if (self->wlr_compositor == NULL) {
    zn_error("Failed to create a wlr_compositor");
    goto err_free;
  }

  for (int i = 0; i <= 32; i++) {
    sprintf(socket_name_candidate, "wayland-%d", i);  // NOLINT(cert-err33-c)
    if (wl_display_add_socket(display, socket_name_candidate) >= 0) {
      socket = socket_name_candidate;
      break;
    }
  }

  if (socket == NULL) {
    zn_error("Failed to open a wayland socket");
    goto err_free;
  }

  setenv("WAYLAND_DISPLAY", socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->xwayland = wlr_xwayland_create(display, self->wlr_compositor, true);
  if (self->xwayland == NULL) {
    zn_error("Failed to create a wlr_xwayland");
    goto err_free;
  }

  setenv("DISPLAY", self->xwayland->display_name, true);
  zn_debug("DISPLAY=%s", self->xwayland->display_name);

  self->new_xwayland_surface_listener.notify =
      zn_compositor_handle_new_xwayland_surface;
  wl_signal_add(&self->xwayland->events.new_surface,
      &self->new_xwayland_surface_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_compositor_destroy(struct zn_compositor *self)
{
  wl_list_remove(&self->new_xwayland_surface_listener.link);
  wlr_xwayland_destroy(self->xwayland);
  free(self);
}
