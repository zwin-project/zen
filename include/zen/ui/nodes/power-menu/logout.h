#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_power_menu_item_logout {
  struct zigzag_node *zigzag_node;
};

struct zn_power_menu_item_logout *zn_power_menu_item_logout_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_power_menu_item_logout_destroy(struct zn_power_menu_item_logout *self);
