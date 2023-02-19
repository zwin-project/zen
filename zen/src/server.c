#include "zen/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

#include "backend.h"
#include "seat.h"
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

  zn_backend_stop(self->backend);

  self->running = false;
  self->exit_status = exit_status;

  wl_display_terminate(self->display);
}

static void
handle_wlr_log(
    enum wlr_log_importance wlr_importance, const char *fmt, va_list args)
{
  zn_log_importance_t importance = ZEN_DEBUG;

  switch (wlr_importance) {
    case WLR_ERROR:
      importance = ZEN_ERROR;
      break;
    case WLR_INFO:
      importance = ZEN_INFO;
      break;
    default:
      importance = ZEN_DEBUG;
      break;
  }

  int len = snprintf(NULL, 0, "[wlr] %s", fmt);
  char format[len + 1];
  snprintf(format, len + 1, "[wlr] %s", fmt);  // NOLINT(cert-err33-c)

  zn_vlog_(importance, format, args);
}

struct zn_server *
zn_server_create(struct wl_display *display, struct zn_backend *backend)
{
  wlr_log_init(WLR_DEBUG, handle_wlr_log);

  if (!zn_assert(!server_singleton, "zn_server is already initialized")) {
    return NULL;
  }

  struct zn_server *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  server_singleton = self;

  if (backend) {
    self->backend = backend;
  } else {
    self->backend = zn_default_backend_create(display);
  }
  if (self->backend == NULL) {
    zn_error("Failed to create a zn_backend");
    goto err_free;
  }

  self->seat = zn_seat_create();
  if (self->seat == NULL) {
    zn_error("Failed to create a zn_seat");
    goto err_backend;
  }

  self->display = display;
  self->running = false;
  self->exit_status = EXIT_FAILURE;

  return self;

err_backend:
  zn_backend_destroy(self->backend);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  zn_seat_destroy(self->seat);
  zn_backend_destroy(self->backend);
  server_singleton = NULL;
  free(self);
}
