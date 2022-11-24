#include "zen/server.h"

#include <stdio.h>
#include <wayland-server.h>
#include <wlr/render/glew.h>
#include <zen-desktop-protocol.h>
#include <zgnr/virtual-object.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/appearance/system.h"
#include "zen/config.h"
#include "zen/output.h"
#include "zen/scene/virtual-object.h"
#include "zen/xdg-toplevel-view.h"
#include "zen/xwayland-view.h"

static struct zn_server *server_singleton = NULL;

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

static void
zn_server_handle_new_virtual_object(struct wl_listener *listener, void *data)
{
  struct zn_server *self =
      zn_container_of(listener, self, new_virtual_object_listener);
  struct zgnr_virtual_object *zgnr_virtual_object = data;

  (void)zn_virtual_object_create(zgnr_virtual_object, self->scene);
}

static void
zn_server_handle_new_peer(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_peer_listener);
  struct znr_remote_peer *peer = data;

  struct znr_session *session = znr_remote_create_session(self->remote, peer);
  zna_system_set_current_session(self->appearance_system, session);
}

struct zn_server *
zn_server_get_singleton(void)
{
  zn_assert(server_singleton != NULL,
      "zn_server_get_singleton was called before creating zn_server");
  return server_singleton;
}

void
zn_server_change_display_system(
    struct zn_server *self, enum zen_display_system_type display_system)
{
  self->display_system = display_system;
}

int
zn_server_run(struct zn_server *self)
{
  if (!wlr_backend_start(self->wlr_backend)) {
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

  self->zgnr_backend = zgnr_backend_create(self->display);
  if (self->zgnr_backend == NULL) {
    zn_error("Failed to create a zgnr_backend");
    goto err_config;
  }

  self->wlr_backend = wlr_backend_autocreate(self->display);
  if (self->wlr_backend == NULL) {
    zn_error("Failed to create a wlr_backend");
    goto err_zgnr_backend;
  }

  drm_fd = wlr_backend_get_drm_fd(self->wlr_backend);
  if (drm_fd < 0) {
    zn_error("Failed to get drm fd");
    goto err_wlr_backend;
  }

  self->renderer = wlr_glew_renderer_create_with_drm_fd(drm_fd);
  if (self->renderer == NULL) {
    zn_error("Failed to create renderer");
    goto err_wlr_backend;
  }

  if (wlr_renderer_init_wl_display(self->renderer, self->display) == false) {
    zn_error("Failed to initialize renderer with wl_display");
    goto err_renderer;
  }

  self->allocator = wlr_allocator_autocreate(self->wlr_backend, self->renderer);
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

  self->appearance_system = zna_system_create(self->display);
  if (self->appearance_system == NULL) {
    zn_error("Failed to create a zn_appearance");
    goto err_allocator;
  }

  self->scene = zn_scene_create(self->config);
  if (self->scene == NULL) {
    zn_error("Failed to create zn_scene");
    goto err_appearance;
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

  self->shell = zn_shell_create(self->display);
  if (self->shell == NULL) {
    zn_error("Failed to create a zn_shell");
    goto err_server_decoration;
  }

  self->input_manager = zn_input_manager_create(self->display);
  if (self->input_manager == NULL) {
    zn_error("Failed to create input manager");
    goto err_shell;
  }

  self->remote = znr_remote_create(self->display);
  if (self->remote == NULL) {
    zn_error("Failed to create a znr_remote");
    goto err_input_manager;
  }

  self->xwayland = wlr_xwayland_create(self->display, self->w_compositor, true);
  if (self->xwayland == NULL) {
    zn_error("Failed to create xwayland");
    goto err_remote;
  }

  setenv("DISPLAY", self->xwayland->display_name, true);
  zn_debug("DISPLAY=%s", self->xwayland->display_name);

  zn_scene_setup_bindings(self->scene);

  self->new_input_listener.notify = zn_server_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  self->new_output_listener.notify = zn_server_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  self->xdg_shell_new_surface_listener.notify =
      zn_server_handle_xdg_shell_new_surface;
  wl_signal_add(&self->xdg_shell->events.new_surface,
      &self->xdg_shell_new_surface_listener);

  self->xwayland_new_surface_listener.notify =
      zn_server_handle_xwayland_new_surface;
  wl_signal_add(&self->xwayland->events.new_surface,
      &self->xwayland_new_surface_listener);

  self->new_virtual_object_listener.notify =
      zn_server_handle_new_virtual_object;
  wl_signal_add(&self->zgnr_backend->events.new_virtual_object,
      &self->new_virtual_object_listener);

  self->new_peer_listener.notify = zn_server_handle_new_peer;
  wl_signal_add(&self->remote->events.new_peer, &self->new_peer_listener);

  // REMOVE ME LATER:
  zgnr_backend_activate(self->zgnr_backend);

  return self;

err_remote:
  znr_remote_destroy(self->remote);

err_input_manager:
  zn_input_manager_destroy(self->input_manager);

err_shell:
  zn_shell_destroy(self->shell);

err_server_decoration:
  zn_decoration_manager_destroy(self->decoration_manager);

err_socket:
  free(self->socket);

err_scene:
  zn_scene_destroy(self->scene);

err_appearance:
  zna_system_destroy(self->appearance_system);

err_allocator:
  wlr_allocator_destroy(self->allocator);

err_renderer:
  wlr_renderer_destroy(self->renderer);

err_wlr_backend:
  wlr_backend_destroy(self->wlr_backend);

err_zgnr_backend:
  zgnr_backend_destroy(self->zgnr_backend);

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
  wlr_backend_destroy(self->wlr_backend);
  wl_display_destroy_clients(self->display);

  zn_cursor_destroy_resources(self->input_manager->seat->cursor);
  zn_scene_destroy_resources(self->scene);
}

void
zn_server_destroy(struct zn_server *self)
{
  wlr_xwayland_destroy(self->xwayland);
  zna_system_destroy(self->appearance_system);
  znr_remote_destroy(self->remote);
  zn_input_manager_destroy(self->input_manager);
  zn_shell_destroy(self->shell);
  zn_decoration_manager_destroy(self->decoration_manager);
  free(self->socket);
  zn_scene_destroy(self->scene);
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  zgnr_backend_destroy(self->zgnr_backend);
  server_singleton = NULL;
  zn_config_destroy(self->config);
  free(self);
}
