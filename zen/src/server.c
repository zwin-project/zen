#include "zen/server.h"

#include <stdlib.h>
#include <wayland-server-core.h>

#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
static struct zn_server *server_singleton = NULL;

struct zn_server *
zn_server_get_singleton(void)
{
  zn_assert(server_singleton, "zn_server is not initialized yet");
  return server_singleton;
}

int
zn_server_run(struct zn_server *self)
{
  self->running = true;

  if (!zn_backend_start(self->backend)) {
    zn_error("Failed to start zn_backend");
    return EXIT_FAILURE;
  }

  wl_display_run(self->display);

  if (!zn_assert(!self->running, "Not terminated with zn_server_terminate")) {
    return EXIT_FAILURE;
  }

  return self->exit_status;
}

void
zn_server_terminate(struct zn_server *self, int exit_status)
{
  if (!self->running) {
    return;
  }

  self->running = false;
  self->exit_status = exit_status;

  wl_display_terminate(self->display);
}

struct zn_server *
zn_server_create(struct wl_display *display)
{
  if (!zn_assert(!server_singleton, "zn_server is already initialized")) {
    return NULL;
  }

  struct zn_server *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->backend = zn_backend_create(display);
  if (self->backend == NULL) {
    zn_error("Failed to create a zn_backend");
    goto err_free;
  }

  self->display = display;
  self->running = false;
  self->exit_status = EXIT_FAILURE;

  server_singleton = self;

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  zn_backend_destroy(self->backend);
  server_singleton = NULL;
  free(self);
}
