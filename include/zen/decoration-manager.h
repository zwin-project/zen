#pragma once

#include <wayland-server.h>
#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

struct zn_decoration_manager {
  // these objects will be automatically destroyed when wl_display is destroyed
  struct wlr_server_decoration_manager* decoration_manager;
  struct wlr_xdg_decoration_manager_v1* xdg_decoration_manager;

  struct wl_list xdg_decoration_list;  // zn_xdg_decoration::link

  struct wl_listener new_xdg_decoration_listener;
};

struct zn_decoration_manager* zn_decoration_manager_create(
    struct wl_display* display);

void zn_decoration_manager_destroy(struct zn_decoration_manager* self);
