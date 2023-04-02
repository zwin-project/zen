#pragma once

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_output_v1.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

struct zn_compositor {
  // These objects will be automatically destroyed when wl_display is destroyed
  struct wlr_compositor *wlr_compositor;                 // @nonnull, @outlive
  struct wlr_xdg_shell *xdg_shell;                       // @nonnull, @outlive
  struct wlr_layer_shell_v1 *wlr_layer_shell;            // @nonnull, @outlive
  struct wlr_data_device_manager *data_device_manager;   // @nonnull, @outlive
  struct wlr_xdg_output_manager_v1 *xdg_output_manager;  // @nonnull, @outlive

  // TODO(@Aki-7): Use this appropriately #388
  struct wlr_output_layout *output_layout;  // @nonnull, @owning

  struct wlr_xwayland *xwayland;  // @nullable, @owning

  struct wl_listener new_xwayland_surface_listener;
  struct wl_listener new_layer_surface_listener;
  struct wl_listener server_end_listener;
};

struct zn_compositor *zn_compositor_create(
    struct wl_display *display, struct wlr_renderer *renderer);

void zn_compositor_destroy(struct zn_compositor *self);