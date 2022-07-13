#include "server.h"

#include <wayland-server.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_output.h>

#include "zen-common/log.h"
#include "zen-common/util.h"

struct zn_server {
  struct wl_display *display;
  struct wlr_backend *backend;

  struct wl_listener new_output_listener;

  int exit_code;
};

static void
zn_server_new_output_handler(struct wl_listener *listener, void *data)
{
  struct zn_server *self = zn_container_of(listener, self, new_output_listener);
  struct wlr_output *output = data;
  UNUSED(self);
  UNUSED(output);

  // TODO: implement here
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

  self->backend = wlr_backend_autocreate(self->display, NULL);
  if (self->backend == NULL) {
    zn_error("Failed to create a backend");
    goto err_free;
  }

  self->new_output_listener.notify = zn_server_new_output_handler;
  wl_signal_add(&self->backend->events.new_output, &self->new_output_listener);

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  wlr_backend_destroy(self->backend);
  free(self);
}
