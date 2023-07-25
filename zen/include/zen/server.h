#pragma once

#include <stdbool.h>
#include <wayland-server-core.h>

struct zn_backend;
struct zn_seat;
struct zn_binding;
struct zn_inode;

struct zn_server {
  struct wl_display *display;  // @nonnull, @outlive

  struct zn_backend *backend;  // @nonnull, @owning
  struct zn_seat *seat;        // @nonnull, @owning
  struct zn_binding *binding;  // @nonnull, @owning
  struct zn_config *config;    // @nonnull, @outlive

  // zn_inode_set_xr_system for this root inode is called in the backend
  // implementation
  struct zn_inode *inode_root;  // @nonnull, @owning

  bool running;
  int exit_status;

  struct {
    struct wl_signal start;  // (NULL)
    struct wl_signal end;    // (NULL)
  } events;
};

struct zn_server *zn_server_get_singleton(void);

/// @return exit status
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_status);

/// @param backend is nullable, ownership will be moved
/// @param config must not be null
struct zn_server *zn_server_create(struct wl_display *display,
    struct zn_backend *backend, struct zn_config *config);

void zn_server_destroy(struct zn_server *self);
