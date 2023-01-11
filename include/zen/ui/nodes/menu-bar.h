#pragma once

#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_power_button;
struct zn_board_selector;
struct zn_vr_button;

struct zn_menu_bar {
  struct zigzag_node *zigzag_node;

  struct zn_power_button *power_button;
  struct zn_board_selector *board_selector;
  struct wl_list launcher_list;  // zn_app_launcher::link
  struct zn_vr_button *vr_button;
};

struct zn_menu_bar *zn_menu_bar_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, struct zn_screen *screen);

void zn_menu_bar_destroy(struct zn_menu_bar *self);
