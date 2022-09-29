#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

#include "zen/decoration-manager.h"
#include "zen/scene/view.h"

struct zn_xdg_decoration {
  struct wlr_xdg_toplevel_decoration_v1* wlr_decoration;  // nonnull
  struct zn_view* view;                                   // nonnull

  struct wl_listener request_mode_listener;
  struct wl_listener view_destroy_listener;
  struct wl_listener decoration_destroy_listener;
};

struct zn_xdg_decoration* zn_xdg_decoration_create(
    struct wlr_xdg_toplevel_decoration_v1* decoration);

void zn_xdg_decoration_destroy(struct zn_xdg_decoration* self);
