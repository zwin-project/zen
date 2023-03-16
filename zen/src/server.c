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

  wl_signal_emit(&self->events.start, NULL);

  if (!self->running) {
    return self->exit_status;
  }

  wl_display_run(self->display);

  if (!zn_assert(!self->running, "Not terminated with zn_server_terminate")) {
    self->exit_status = EXIT_FAILURE;
  }

  wl_display_destroy_clients(self->display);

  wl_signal_emit(&self->events.end, NULL);

  zn_backend_stop(self->backend);

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
  wl_signal_init(&self->events.start);
  wl_signal_init(&self->events.end);
  self->display = display;
  self->running = false;
  self->exit_status = EXIT_FAILURE;

  self->seat = zn_seat_create(display);
  if (self->seat == NULL) {
    zn_error("Failed to create a zn_seat");
    goto err_free;
  }

  if (backend) {
    self->backend = backend;
  } else {
    self->backend = zn_default_backend_create(display, self->seat);
  }
  if (self->backend == NULL) {
    zn_error("Failed to create a zn_backend");
    goto err_seat;
  }

  return self;

err_seat:
  zn_seat_destroy(self->seat);

err_free:
  free(self);

err:
  return NULL;
}

void
zn_server_destroy(struct zn_server *self)
{
  zn_backend_destroy(self->backend);
  zn_seat_destroy(self->seat);
  server_singleton = NULL;
  wl_list_remove(&self->events.start.listener_list);
  wl_list_remove(&self->events.end.listener_list);
  free(self);
}
