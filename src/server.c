#include "server.h"

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>

#include "output.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_server {
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;

  struct wl_listener new_output_listener;

  int exit_code;
};

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

struct wl_event_loop *
zn_server_get_loop(struct zn_server *self)
{
  return self->loop;
}

struct wlr_renderer *
zn_server_get_renderer(struct zn_server *self)
{
  return self->renderer;
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

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

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

  self->allocator = wlr_allocator_autocreate(self->backend, self->renderer);
  if (self->allocator == NULL) {
    zn_error("Failed to create allocator");
    goto err_renderer;
  }

  self->new_output_listener.notify = zn_server_new_output_handler;
  wl_signal_add(&self->backend->events.new_output, &self->new_output_listener);

  return self;

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
  wlr_allocator_destroy(self->allocator);
  wlr_renderer_destroy(self->renderer);
  wlr_backend_destroy(self->backend);
  free(self);
}
