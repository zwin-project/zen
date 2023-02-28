#pragma once

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/xwayland.h>

struct zn_compositor {
  // This object will be automatically destroyed when wl_display is destroyed
  struct wlr_compositor *wlr_compositor;  // @nonnull, @outlive

  struct wlr_xwayland *xwayland;  // @nonnull, @owning

  struct wl_listener new_xwayland_surface_listener;
};

struct zn_compositor *zn_compositor_create(
    struct wl_display *display, struct wlr_renderer *renderer);

void zn_compositor_destroy(struct zn_compositor *self);
