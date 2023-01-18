#pragma once

#include <wlr/render/wlr_renderer.h>

#include "zigzag/node.h"

struct zn_power_menu_item_clock {
  struct zigzag_node *zigzag_node;

  // TODO: Prepare a common struct for power_menu_item_clock and power_button
  struct wl_event_source *second_timer_source;
};

struct zn_power_menu_item_clock *zn_power_menu_item_clock_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_power_menu_item_clock_destroy(struct zn_power_menu_item_clock *self);
