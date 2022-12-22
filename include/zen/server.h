#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <zgnr/backend.h>
#include <znr-remote.h>

#include "zen/appearance/system.h"
#include "zen/config/config.h"
#include "zen/data_device_manager.h"
#include "zen/input/input-manager.h"
#include "zen/scene.h"
#include "zen/screen/compositor.h"
#include "zen/shell/shell.h"

enum zn_display_system_state {
  ZN_DISPLAY_SYSTEM_SCREEN,
  ZN_DISPLAY_SYSTEM_IMMERSIVE,
};

struct zn_server {
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wlr_backend *wlr_backend;
  struct zgnr_backend *zgnr_backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;

  struct zn_screen_compositor *screen_compositor;

  struct zn_data_device_manager *data_device_manager;
  struct zn_input_manager *input_manager;
  struct znr_remote *remote;
  struct zna_system *appearance_system;
  struct zn_shell *shell;
  struct zn_scene *scene;    // nonnull
  struct zn_config *config;  // nonnull

  enum zn_display_system_state display_system;

  char *socket;

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
  struct wl_listener new_virtual_object_listener;
  struct wl_listener new_peer_listener;

  int exit_code;
};

struct zn_server *zn_server_get_singleton(void);

void zn_server_change_display_system(
    struct zn_server *self, enum zn_display_system_state display_system);

/** returns exit code */
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_code);

struct zn_server *zn_server_create(struct wl_display *display);

void zn_server_destroy_resources(struct zn_server *self);

void zn_server_destroy(struct zn_server *self);
