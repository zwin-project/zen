
#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_power_menu_item_clock;
struct zn_power_menu_item_logout;

struct zn_power_menu {
  struct zigzag_node *zigzag_node;

  double tip_x;

  struct zn_power_menu_item_clock *item_clock;
  struct zn_power_menu_item_logout *item_logout;
};

struct zn_power_menu *zn_power_menu_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, double tip_x);

void zn_power_menu_destroy(struct zn_power_menu *self);
