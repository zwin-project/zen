#include "zen/server.h"

#include <stdio.h>
#include <wayland-server.h>
#include <wlr/render/glew.h>
#include <zen-desktop-protocol.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/config.h"
#include "zen/output.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

static struct zn_server *server_singleton = NULL;

static void
zn_server_handle_immersive_activate(struct wl_listener *listener, void *data)
{
  struct zn_server *self =
      zn_container_of(listener, self, immersive_activate_listener);
  UNUSED(data);

  self->display_system = ZEN_DISPLAY_SYSTEM_TYPE_IMMERSIVE;
}

static void
zn_server_handle_immersive_deactivated(struct wl_listener *listener, void *data)
{
  struct zn_server *self =
      zn_container_of(listener, self, immersive_deactivated_listener);
  UNUSED(data);

  self->display_system = ZEN_DISPLAY_SYSTEM_TYPE_SCREEN;
}

static void
zn_server_handle_new_input(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *wlr_input = data;
  zn_input_manager_handle_new_wlr_input(self->input_manager, wlr_input);
}

static void
zn_server_handle_new_output(struct wl_listener *listener, void *data)
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
zn_server_handle_xdg_shell_new_surface(struct wl_listener *listener, void *data)
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
zn_server_handle_xwayland_new_surface(struct wl_listener *listener, void *data)
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
  int drm_fd = -1;

  if (!zn_assert(server_singleton == NULL,
          "Tried to create zn_server multiple times")) {
    goto err;
  }

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->config = zn_config_create();
  if (self->config == NULL) {
    zn_error("Failed to create a config object");
    goto err_free;
  }

  server_singleton = self;
  self->display = display;
  self->exit_code = EXIT_FAILURE;
  self->loop = wl_display_get_event_loop(display);
  self->display_system = ZEN_DISPLAY_SYSTEM_TYPE_SCREEN;

  self->backend = wlr_backend_autocreate(self->display);
  if (self->backend == NULL) {
    zn_error("Failed to create a backend");
    goto err_config;
  }

  drm_fd = wlr_backend_get_drm_fd(self->backend);
  if (drm_fd < 0) {
    zn_error("Failed to get drm fd");
    goto err_config;
  }

  self->renderer = wlr_glew_renderer_create_with_drm_fd(drm_fd);
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

  self->scene = zn_scene_create(self->config);
  if (self->scene == NULL) {
    zn_error("Failed to create zn_scene");
    goto err_allocator;
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
    goto err_scene;
  }

  self->decoration_manager = zn_decoration_manager_create(self->display);
  if (self->decoration_manager == NULL) {
    zn_error("Failed to create server decoration");
    goto err_socket;
  }

  setenv("WAYLAND_DISPLAY", self->socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", self->socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->input_manager = zn_input_manager_create(self->display);
  if (self->input_manager == NULL) {
    zn_error("Failed to create input manager");
    goto err_server_decoration;
  }

  self->remote = znr_remote_create(self->display);
  if (self->remote == NULL) {
    zn_error("Failed to create znr_remote");
    goto err_input_manager;
  }

  self->remote_renderer =
      zn_remote_immersive_renderer_create(self->scene, self->remote);
  if (self->remote_renderer == NULL) {
    zn_error("Failed to create a remote immersive renderer");
    goto err_remote;
  }

  self->immersive_display_system = zn_remote_immersive_display_system_create(
      self->display, self->remote_renderer, self->remote);
  if (self->immersive_display_system == NULL) {
    zn_error("Failed to create remote immersive display system");
    goto err_remote_renderer;
  }

  self->xwayland = wlr_xwayland_create(self->display, self->w_compositor, true);
  if (self->xwayland == NULL) {
    zn_error("Failed to create xwayland");
    goto err_immersive_display_system;
  }

  setenv("DISPLAY", self->xwayland->display_name, true);
  zn_debug("DISPLAY=%s", self->xwayland->display_name);

  zn_scene_setup_bindings(self->scene);

  self->immersive_activate_listener.notify =
      zn_server_handle_immersive_activate;
  wl_signal_add(&self->immersive_display_system->events.activate,
      &self->immersive_activate_listener);

  self->immersive_deactivated_listener.notify =
      zn_server_handle_immersive_deactivated;
  wl_signal_add(&self->immersive_display_system->events.deactivated,
      &self->immersive_deactivated_listener);

  self->new_input_listener.notify = zn_server_handle_new_input;
  wl_signal_add(&self->backend->events.new_input, &self->new_input_listener);

  self->new_output_listener.notify = zn_server_handle_new_output;
  wl_signal_add(&self->backend->events.new_output, &self->new_output_listener);

  self->xdg_shell_new_surface_listener.notify =
      zn_server_handle_xdg_shell_new_surface;
  wl_signal_add(&self->xdg_shell->events.new_surface,
      &self->xdg_shell_new_surface_listener);

  self->xwayland_new_surface_listener.notify =
      zn_server_handle_xwayland_new_surface;
  wl_signal_add(&self->xwayland->events.new_surface,
      &self->xwayland_new_surface_listener);

  return self;

err_immersive_display_system:
  zn_remote_immersive_display_system_destroy(self->immersive_display_system);

err_remote_renderer:
  zn_remote_immersive_renderer_destroy(self->remote_renderer);

err_remote:
  znr_remote_destroy(self->remote);

err_input_manager:
  zn_input_manager_destroy(self->input_manager);

err_server_decoration:
  zn_decoration_manager_destroy(self->decoration_manager);

err_socket:
  free(self->socket);

err_scene:
  zn_scene_destroy(self->scene);

err_allocator:
  wlr_allocator_destroy(self->allocator);

err_renderer:
  wlr_renderer_destroy(self->renderer);

err_backend:
  wlr_backend_destroy(self->backend);

err_config:
  zn_config_destroy(self->config);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy_resources(struct zn_server *self)
{
  wlr_backend_destroy(self->backend);
  wl_display_destroy_clients(self->display);

  zn_cursor_destroy_resources(self->input_manager->seat->cursor);
  zn_scene_destroy_resources(self->scene);

  znr_remote_stop(self->remote);
}

void
zn_server_destroy(struct zn_server *self)
{
  wlr_xwayland_destroy(self->xwayland);
  zn_remote_immersive_display_system_destroy(self->immersive_display_system);
  zn_remote_immersive_renderer_destroy(self->remote_renderer);
  znr_remote_destroy(self->remote);
  zn_input_manager_destroy(self->input_manager);
  zn_decoration_manager_destroy(self->decoration_manager);
  free(self->socket);
  zn_scene_destroy(self->scene);
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  server_singleton = NULL;
  zn_config_destroy(self->config);
  free(self);
}
