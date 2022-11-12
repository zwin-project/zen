#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

#include "zen/scene/screen.h"
#include "zen/server.h"

struct zn_ui_node;

typedef void (*zn_ui_node_on_click_handler_t)(
    struct zn_ui_node *self, double x, double y);

struct zn_ui_node {
  struct wlr_box *frame;
  struct wlr_texture *texture;

  zn_ui_node_on_click_handler_t handler;

  struct wl_list link;
  struct wl_list children;
};

void zn_ui_node_setup_default(
    struct zn_screen *screen, struct zn_server *server);