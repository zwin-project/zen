#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

#include "zen/decoration-manager.h"
#include "zen/scene/view.h"

struct zn_xdg_decoration {
  struct wlr_xdg_toplevel_decoration_v1* wlr_decoration;
  struct zn_view* view;
  struct wl_list link;  // zn_decoration_manager::xdg_decoration_list

  struct wl_listener request_mode_listener;
  struct wl_listener view_destroy_listener;
  struct wl_listener decoration_destroy_listener;
};

struct zn_xdg_decoration* zn_xdg_decoration_create(
    struct zn_decoration_manager* manager,
    struct wlr_xdg_toplevel_decoration_v1* decoration);

void zn_xdg_decoration_destory(struct zn_xdg_decoration* self);
