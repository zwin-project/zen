#pragma once

#include <wlr/types/wlr_xdg_decoration_v1.h>

struct zn_xdg_decoration {
  struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration;  // @nonnull, @outlive

  struct wl_listener wlr_decoration_destroy_listener;
  struct wl_listener wlr_decoration_request_mode_listener;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

struct zn_xdg_decoration *zn_xdg_decoration_create(
    struct wlr_xdg_toplevel_decoration_v1 *wlr_decoration);
