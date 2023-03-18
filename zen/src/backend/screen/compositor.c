#include "compositor.h"

#include <wlr/types/wlr_layer_shell_v1.h>

#include "layer-shell.h"
#include "layer-surface.h"
#include "output.h"
#include "xwayland-surface.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/seat.h"
#include "zen/server.h"

static void
zn_compositor_handle_new_xwayland_surface(
    struct wl_listener *listener UNUSED, void *data)
{
  struct wlr_xwayland_surface *wlr_xsurface = data;

  if (wlr_xsurface->override_redirect) {
    zn_debug("skip unmanaged surface");
    // TODO(@Aki-7): handle xwayland surface with override_redirect flag
    return;
  }

  zn_xwayland_surface_create(wlr_xsurface);
}

static void
zn_compositor_handle_new_layer_surface(
    struct wl_listener *listener UNUSED, void *data)
{
  struct wlr_layer_surface_v1 *wlr_layer_surface = data;
  struct zn_output *output = NULL;
  struct zn_server *server = zn_server_get_singleton();

  if (wlr_layer_surface->output) {
    output = zn_output_get(wlr_layer_surface->output);
  } else {
    struct zn_screen *screen = zn_seat_get_focused_screen(server->seat);
    if (screen) {
      output = zn_output_from_screen(screen);
    }
  }

  if (output == NULL) {
    zn_warn("No output to auto-assign layer surface '%s' to",
        wlr_layer_surface->namespace);
    wlr_layer_surface_v1_destroy(wlr_layer_surface);
    return;
  }

  struct zn_layer_surface *layer_surface =
      zn_layer_surface_create(wlr_layer_surface, output);
  if (layer_surface == NULL) {
    zn_warn("Failed to create a layer surface");
    wlr_layer_surface_v1_destroy(wlr_layer_surface);
    return;
  }

  zn_layer_shell_add_layer_surface(
      output->layer_shell, layer_surface, wlr_layer_surface->current.layer);
}

static void
zn_compositor_handle_server_end(struct wl_listener *listener, void *data UNUSED)
{
  struct zn_compositor *self =
      zn_container_of(listener, self, server_end_listener);

  if (self->xwayland) {
    wlr_xwayland_destroy(self->xwayland);
    self->xwayland = NULL;
  }
}

struct zn_compositor *
zn_compositor_create(struct wl_display *display, struct wlr_renderer *renderer)
{
  char socket_name_candidate[16];
  const char *socket = NULL;
  const char *xdg = NULL;
  struct zn_server *server = zn_server_get_singleton();

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

  self->xdg_shell = wlr_xdg_shell_create(display);
  if (self->xdg_shell == NULL) {
    zn_error("Failed to create a xdg_shell");
    goto err_free;
  }

  self->wlr_layer_shell = wlr_layer_shell_v1_create(display);
  if (self->wlr_layer_shell == NULL) {
    zn_error("Failed to create a wlr_layer_shell");
    goto err_free;
  }

  self->data_device_manager = wlr_data_device_manager_create(display);
  if (self->data_device_manager == NULL) {
    zn_error("Failed to create a wlr_data_device_manager");
    goto err_free;
  }

  self->output_layout = wlr_output_layout_create();
  if (self->output_layout == NULL) {
    zn_error("Failed to create a wlr_output_layout");
    goto err_free;
  }

  self->xdg_output_manager =
      wlr_xdg_output_manager_v1_create(display, self->output_layout);
  if (self->xdg_output_manager == NULL) {
    zn_error("Failed to create a wlr_xdg_output_manager");
    goto err_output_layout;
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
    goto err_output_layout;
  }

  setenv("WAYLAND_DISPLAY", socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->xwayland = wlr_xwayland_create(display, self->wlr_compositor, true);
  if (self->xwayland == NULL) {
    zn_error("Failed to create a wlr_xwayland");
    goto err_output_layout;
  }

  setenv("DISPLAY", self->xwayland->display_name, true);
  zn_debug("DISPLAY=%s", self->xwayland->display_name);

  self->new_xwayland_surface_listener.notify =
      zn_compositor_handle_new_xwayland_surface;
  wl_signal_add(&self->xwayland->events.new_surface,
      &self->new_xwayland_surface_listener);

  self->new_layer_surface_listener.notify =
      zn_compositor_handle_new_layer_surface;
  wl_signal_add(&self->wlr_layer_shell->events.new_surface,
      &self->new_layer_surface_listener);

  self->server_end_listener.notify = zn_compositor_handle_server_end;
  wl_signal_add(&server->events.end, &self->server_end_listener);

  return self;

err_output_layout:
  wlr_output_layout_destroy(self->output_layout);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_compositor_destroy(struct zn_compositor *self)
{
  wl_list_remove(&self->server_end_listener.link);
  wl_list_remove(&self->new_layer_surface_listener.link);
  wl_list_remove(&self->new_xwayland_surface_listener.link);
  if (self->xwayland) {
    wlr_xwayland_destroy(self->xwayland);
  }
  wlr_output_layout_destroy(self->output_layout);
  free(self);
}
