#pragma once

#include <cairo.h>
#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/ui/nodes/power-menu/power-menu.h"

struct zn_power_button {
  struct zigzag_node *zigzag_node;

  struct wl_event_source *second_timer_source;

  cairo_surface_t *power_icon_surface;

  struct zn_power_menu *power_menu;
};

struct zn_power_button *zn_power_button_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_power_button_destroy(struct zn_power_button *self);
