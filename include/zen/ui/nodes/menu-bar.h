#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zen/ui/nodes/power-button.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_menu_bar {
  struct zigzag_node *zigzag_node;

  struct zn_power_button *power_button;
};

struct zn_menu_bar *zn_menu_bar_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_menu_bar_destroy(struct zn_menu_bar *self);
