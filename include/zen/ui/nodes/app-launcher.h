#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_favorite_app;

struct zn_app_launcher {
  struct zigzag_node *zigzag_node;

  struct zn_favorite_app *app;  // not owning

  struct wl_list link;  // for zn_menu_bar::launcher_list
  cairo_surface_t *launcher_icon_surface;
  int idx;
};

struct zn_app_launcher *zn_app_launcher_create(
    struct zigzag_layout *zigzag_layout, struct zn_favorite_app *app, int idx);

void zn_app_launcher_destroy(struct zn_app_launcher *self);
