#include "zen/server.h"

#include <stdio.h>
#include <wlr/render/glew.h>
#include <wlr/types/wlr_output.h>
#include <zen-common.h>

#include "zen/screen/output.h"

static struct zn_server *server_singleton = NULL;

static void
zn_server_handle_new_input(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_input_listener);
  struct wlr_input_device *wlr_input = data;
  UNUSED(self);
  UNUSED(wlr_input);
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
    zn_debug("Skip non-desktop output");
    // TODO: DRM lease support for VR headsets
    return;
  }

  if (wlr_output_init_render(wlr_output, self->allocator, self->renderer) ==
      false) {
    zn_abort("Failed to initialize output renderer");
    return;
  }

  output = zn_output_create(wlr_output);
  if (output == NULL) {
    zn_error("Failed to create a zn_output");
    return;
  }
}

struct zn_server *
zn_server_get_singleton(void)
{
  zn_assert(server_singleton != NULL,
      "zn_server_get_singleton was called before creating zn_server");
  return server_singleton;
}

/** returns exit code */
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

  server_singleton = self;
  self->display = display;
  self->exit_code = EXIT_FAILURE;
  self->loop = wl_display_get_event_loop(display);

  self->wlr_backend = wlr_backend_autocreate(self->display);
  if (self->wlr_backend == NULL) {
    zn_error("Failed to create a wlr_backend");
    goto err_free;
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

  self->scene = zn_scene_create();
  if (self->scene == NULL) {
    zn_error("Failed to create a zn_scene");
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

  setenv("WAYLAND_DISPLAY", self->socket, true);
  xdg = getenv("XDG_RUNTIME_DIR");
  zn_debug("WAYLAND_DISPLAY=%s", self->socket);
  zn_debug("XDG_RUNTIME_DIR=%s", xdg);

  self->new_input_listener.notify = zn_server_handle_new_input;
  wl_signal_add(
      &self->wlr_backend->events.new_input, &self->new_input_listener);

  self->new_output_listener.notify = zn_server_handle_new_output;
  wl_signal_add(
      &self->wlr_backend->events.new_output, &self->new_output_listener);

  return self;

err_scene:
  zn_scene_destroy(self->scene);

err_allocator:
  wlr_allocator_destroy(self->allocator);

err_renderer:
  wlr_renderer_destroy(self->renderer);

err_wlr_backend:
  wlr_backend_destroy(self->wlr_backend);

err_free:
  server_singleton = NULL;
  free(self);

err:
  return NULL;
}

void
zn_server_destroy_resources(struct zn_server *self)
{
  wlr_backend_destroy(self->wlr_backend);
  wl_display_destroy_clients(self->display);
}

void
zn_server_destroy(struct zn_server *self)
{
  free(self->socket);
  zn_scene_destroy(self->scene);
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  server_singleton = NULL;
  free(self);
}
