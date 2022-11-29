#pragma once

#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

#include "zen/scene/screen.h"
#include "zen/server.h"

struct zn_ui_node;

typedef void (*zn_ui_node_on_click_handler_t)(
    struct zn_ui_node *self, double x, double y);

typedef void (*zn_ui_node_render_t)(struct zn_ui_node *self, cairo_t *cr);

struct zn_ui_node {
  struct wlr_box *frame;
  struct wlr_texture *texture;

  struct zn_screen *screen;

  zn_ui_node_on_click_handler_t on_click_handler;
  zn_ui_node_render_t renderer;

  struct wl_list link;
  struct wl_list children;

  void *data;
};

void zn_ui_node_setup_default(
    struct zn_screen *screen, struct zn_server *server);