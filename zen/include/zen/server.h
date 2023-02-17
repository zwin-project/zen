#pragma once

#include <stdbool.h>
#include <wayland-server-core.h>

struct zn_backend;
struct zn_seat;

struct zn_server {
  struct wl_display *display;  // @nonnull, @outlive

  struct zn_backend *backend;  // @nonnull, @owning
  struct zn_seat *seat;        // @nonnull, @owning

  bool running;
  int exit_status;
};

struct zn_server *zn_server_get_singleton(void);

/// @return exit status
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_status);

/// @param backend is nullable, ownership will be moved
struct zn_server *zn_server_create(
    struct wl_display *display, struct zn_backend *backend);

void zn_server_destroy(struct zn_server *self);
