#pragma once

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>
#include <zen-desktop-protocol.h>
#include <znr/remote.h>

#include "zen/config.h"
#include "zen/decoration-manager.h"
#include "zen/immersive/remote/display-system.h"
#include "zen/immersive/remote/renderer.h"
#include "zen/input/input-manager.h"
#include "zen/scene/scene.h"

struct zn_server {
  struct wl_display *display;
  struct wl_event_loop *loop;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;
  struct wlr_xwayland *xwayland;

  // these objects will be automatically destroyed when wl_display is destroyed
  struct wlr_compositor *w_compositor;
  struct wlr_xdg_shell *xdg_shell;
  struct zn_decoration_manager *decoration_manager;

  struct zn_config *config;
  struct zn_input_manager *input_manager;
  struct znr_remote *remote;
  struct zn_remote_immersive_renderer *remote_renderer;
  struct zn_immersive_display_system *immersive_display_system;

  struct zn_scene *scene;

  enum zen_display_system_type display_system;

  char *socket;

  struct wl_listener immersive_activate_listener;
  struct wl_listener immersive_deactivated_listener;
  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
  struct wl_listener xdg_shell_new_surface_listener;
  struct wl_listener xwayland_new_surface_listener;

  int exit_code;
};

struct zn_server *zn_server_get_singleton(void);

/** returns exit code */
int zn_server_run(struct zn_server *self);

void zn_server_terminate(struct zn_server *self, int exit_code);

struct zn_server *zn_server_create(struct wl_display *display);

void zn_server_destroy_resources(struct zn_server *self);

void zn_server_destroy(struct zn_server *self);
