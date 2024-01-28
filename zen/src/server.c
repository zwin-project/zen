#include "zen/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <wayland-server-core.h>
#include <wlr/util/log.h>

#include "backend.h"
#include "binding.h"
#include "seat.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen/backend.h"
#include "zen/config.h"
#include "zen/inode.h"

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

  zn_binding_remap(self->binding);

  wl_display_run(self->display);

  if (!zn_assert(!self->running, "Not terminated with zn_server_terminate")) {
    self->exit_status = EXIT_FAILURE;
  }

  wl_signal_emit(&self->events.end, NULL);

  wl_display_destroy_clients(self->display);

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
zn_server_create(struct wl_display *display, struct zn_backend *backend,
    struct zn_config *config)
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
  self->config = config;

  self->binding = zn_binding_create();
  if (self->binding == NULL) {
    zn_error("Failed to create a zn_binding");
    goto err_free;
  }

  self->seat = zn_seat_create(display);
  if (self->seat == NULL) {
    zn_error("Failed to create a zn_seat");
    goto err_binding;
  }

  self->inode_root = zn_inode_create(self, &zn_inode_noop_implementation);
  if (self->inode_root == NULL) {
    zn_error("Failed to create a root inode");
    goto err_seat;
  }

  self->inode_invisible_root =
      zn_inode_create(self, &zn_inode_noop_implementation);
  if (self->inode_invisible_root == NULL) {
    zn_error("Failed to create a root inode");
    goto err_inode_root;
  }

  if (backend) {
    self->backend = backend;
  } else {
    self->backend = zn_default_backend_create(display, self->seat);
  }
  if (self->backend == NULL) {
    zn_error("Failed to create a zn_backend");
    goto err_inode_invisible_root;
  }

  return self;

err_inode_invisible_root:
  zn_inode_destroy(self->inode_invisible_root);

err_inode_root:
  zn_inode_destroy(self->inode_root);

err_seat:
  zn_seat_destroy(self->seat);

err_binding:
  zn_binding_destroy(self->binding);

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
  zn_inode_destroy(self->inode_root);
  zn_inode_destroy(self->inode_invisible_root);
  zn_binding_destroy(self->binding);
  server_singleton = NULL;
  wl_list_remove(&self->events.start.listener_list);
  wl_list_remove(&self->events.end.listener_list);
  free(self);
}
