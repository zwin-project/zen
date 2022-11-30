#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <znr-remote.h>

#include "zen/scene.h"

struct zn_server {
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wlr_backend *wlr_backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;

  struct znr_remote *remote;

  struct zn_scene *scene;  // nonnull

  char *socket;

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
  struct wl_listener new_peer_listener;

  int exit_code;
};

struct zn_server *zn_server_get_singleton(void);

/** returns exit code */
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_code);

struct zn_server *zn_server_create(struct wl_display *display);

void zn_server_destroy_resources(struct zn_server *self);

void zn_server_destroy(struct zn_server *self);
