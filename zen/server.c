#include "zen/server.h"

#include <stdio.h>
#include <wayland-server.h>
#include <zen-desktop-protocol.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/output.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

static struct zn_server *server_singleton = NULL;

static void
zn_server_new_input_handler(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *wlr_input = data;
  zn_input_manager_handle_new_wlr_input(self->input_manager, wlr_input);
}

static void
zn_server_new_output_handler(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_output_listener);
  struct wlr_output *wlr_output = data;
  struct zn_output *output;

  zn_debug("New output %p: %s (non-desktop: %d)", (void *)wlr_output,
      wlr_output->name, wlr_output->non_desktop);

  if (wlr_output->non_desktop) {
    zn_debug("Not configuring non-desktop output");
    // TODO: DRM lease support for VR headsets
    return;
  }

  if (wlr_output_init_render(wlr_output, self->allocator, self->renderer) ==
      false) {
    zn_abort("Failed to initialize output renderer");
    return;
  }

  output = zn_output_create(wlr_output, self);
  if (output == NULL) {
    zn_warn("Failed to create a zen output");
    return;
  }
  UNUSED(output);
}

static void
zn_server_xdg_shell_new_surface_handler(
    struct wl_listener *listener, void *data)
{
  struct wlr_xdg_surface *xdg_surface = data;
  struct zn_server *self =
      zn_container_of(listener, self, xdg_shell_new_surface_listener);

  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
    zn_debug("New xdg_shell popup");
    return;
  }

  zn_debug("New xdg_shell toplevel title='%s' app_id='%s'",
      xdg_surface->toplevel->title, xdg_surface->toplevel->app_id);
  wlr_xdg_surface_ping(xdg_surface);

  (void)zn_xdg_toplevel_view_create(xdg_surface->toplevel, self);
}

static void
zn_server_display_system_switch_handler(
    struct wl_listener *listener, void *data)
{
  struct zn_server *self =
      zn_container_of(listener, self, display_system_switch_listener);
  struct zn_display_system_switch_event *event = data;

  if (self->display_system->type == event->type) return;

  if (event->type == ZEN_DISPLAY_SYSTEM_TYPE_IMMERSIVE) {
    if (zn_immersive_backend_connect(self->immersive_backend) == false) {
      zen_display_system_send_warning(event->issuer,
          ZEN_DISPLAY_SYSTEM_WARNING_NO_IMMERSIVE_DEVICE,
          "No available immersive device found");
      zn_warn("Failed to connect to a immersive backend");
      return;
    }
  } else {
    zn_immersive_backend_disconnect(self->immersive_backend);
  }

  // TODO: implement here

  zn_display_system_applied(self->display_system, event->type);
}

static void
zn_server_xwayland_new_surface_handler(struct wl_listener *listener, void *data)
{
  struct wlr_xwayland_surface *xwayland_surface = data;
  struct zn_server *self =
      zn_container_of(listener, self, xwayland_new_surface_listener);

  zn_debug("New xwayland surface title='%s' class='%s'",
      xwayland_surface->title, xwayland_surface->class);
  wlr_xwayland_surface_ping(xwayland_surface);

  (void)zn_xwayland_view_create(xwayland_surface, self);
}

struct zn_server *
zn_server_get_singleton(void)
{
  zn_assert(server_singleton != NULL,
      "zn_server_get_singleton was called before creating zn_server");
  return server_singleton;
}

int
zn_server_run(struct zn_server *self)
{
  self->xwayland = wlr_xwayland_create(self->display, self->w_compositor, true);
  if (self->xwayland == NULL) {
    zn_error("Failed to create xwayland");
    return EXIT_FAILURE;
  }

  self->xwayland_new_surface_listener.notify =
      zn_server_xwayland_new_surface_handler;
  wl_signal_add(&self->xwayland->events.new_surface,
      &self->xwayland_new_surface_listener);

  setenv("DISPLAY", self->xwayland->display_name, true);
  zn_debug("DISPLAY=%s", self->xwayland->display_name);

  if (!wlr_backend_start(self->backend)) {
    zn_error("Failed to start backend");
    return EXIT_FAILURE;
  }

  wl_display_run(self->display);
  return self->exit_code;
}

void
zn_server_terminate(struct zn_server *self, int exit_code)
{
  self->exit_code = exit_code;
  wl_display_terminate(self->display);
}

struct zn_server *
zn_server_create(struct wl_display *display)
{
  struct zn_server *self;
  char socket_name_candidate[16];
  char *xdg;

  if (!zn_assert(server_singleton == NULL,
          "Tried to create zn_server multiple times")) {
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  server_singleton = self;
  self->display = display;
  self->exit_code = EXIT_FAILURE;
  self->loop = wl_display_get_event_loop(display);

  self->backend = wlr_backend_autocreate(self->display);
  if (self->backend == NULL) {
    zn_error("Failed to create a backend");
    goto err_free;
  }

  self->renderer = wlr_renderer_autocreate(self->backend);
  if (self->renderer == NULL) {
    zn_error("Failed to create renderer");
    goto err_backend;
  }

  if (wlr_renderer_init_wl_display(self->renderer, self->display) == false) {
    zn_error("Failed to initialize renderer with wl_display");
    goto err_renderer;
  }

  self->allocator = wlr_allocator_autocreate(self->backend, self->renderer);
  if (self->allocator == NULL) {
    zn_error("Failed to create allocator");
    goto err_renderer;
  }

  self->w_compositor = wlr_compositor_create(self->display, self->renderer);
  if (self->w_compositor == NULL) {
    zn_error("Failed to create wlr_compositor");
    goto err_allocator;
  }

  self->xdg_shell = wlr_xdg_shell_create(self->display);
  if (self->display == NULL) {
    zn_error("Failed to create wlr_xdg_shell");
    goto err_allocator;
  }

  self->scene = zn_scene_create();
  if (self->scene == NULL) {
    zn_error("Failed to create zn_scene");
    goto err_allocator;
  }

  self->display_system = zn_display_system_create(self->display);
  if (self->display_system == NULL) {
    zn_error("Failed to create display system");
    goto err_scene;
  }

  self->immersive_backend = zn_immersive_backend_create();
  if (self->immersive_backend == NULL) {
    zn_error("Failed to create a immersive backend");
    goto err_display_system;
  }

  for (int i = 1; i <= 32; i++) {
    sprintf(socket_name_candidate, "wayland-%d", i);
    if (wl_display_add_socket(self->display, socket_name_candidate) >= 0) {
      self->socket = strdup(socket_name_candidate);
      break;
    }
  }

  if (self->socket == NULL) {
    zn_error("Failed to open wayland socket");
    goto err_immersive_backend;
  }

  self->input_manager = zn_input_manager_create(self->display);
  if (self->input_manager == NULL) {
    zn_error("Failed to create input manager");
    goto err_socket;
  }

  setenv("WAYLAND_DISPLAY", self->socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", self->socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->new_input_listener.notify = zn_server_new_input_handler;
  wl_signal_add(&self->backend->events.new_input, &self->new_input_listener);

  self->new_output_listener.notify = zn_server_new_output_handler;
  wl_signal_add(&self->backend->events.new_output, &self->new_output_listener);

  self->xdg_shell_new_surface_listener.notify =
      zn_server_xdg_shell_new_surface_handler;
  wl_signal_add(&self->xdg_shell->events.new_surface,
      &self->xdg_shell_new_surface_listener);

  self->display_system_switch_listener.notify =
      zn_server_display_system_switch_handler;
  wl_signal_add(&self->display_system->switch_signal,
      &self->display_system_switch_listener);

  return self;

err_socket:
  free(self->socket);

err_immersive_backend:
  zn_immersive_backend_destroy(self->immersive_backend);

err_display_system:
  zn_display_system_destroy(self->display_system);

err_scene:
  zn_scene_destroy(self->scene);

err_allocator:
  wlr_allocator_destroy(self->allocator);

err_renderer:
  wlr_renderer_destroy(self->renderer);

err_backend:
  wlr_backend_destroy(self->backend);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  if (self->xwayland) wlr_xwayland_destroy(self->xwayland);
  zn_input_manager_destroy(self->input_manager);
  zn_display_system_destroy(self->display_system);
  free(self->socket);
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  wlr_backend_destroy(self->backend);
  zn_scene_destroy(self->scene);
  server_singleton = NULL;
  free(self);
}
