#pragma once

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

struct zn_screen_compositor {
  struct wl_display *display;

  // These objects will be automatically destroyed when wl_display is destroyed
  struct wlr_compositor *compositor;
  struct wlr_xdg_shell *xdg_shell;
  struct wlr_xwayland *xwayland;

  struct wl_listener xdg_shell_new_surface_listener;
  struct wl_listener xwayland_ready_listener;
  struct wl_listener xwayland_new_surface_listener;
};

struct zn_screen_compositor *zn_screen_compositor_create(
    struct wl_display *display, struct wlr_renderer *renderer);

void zn_screen_compositor_destroy(struct zn_screen_compositor *self);
