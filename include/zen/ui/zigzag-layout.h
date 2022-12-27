#pragma once

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <zigzag.h>

#include "zen/ui/nodes/menu-bar.h"

struct zn_zigzag_layout {
  struct zigzag_layout *zigzag_layout;

  struct wlr_output_damage *damage;
  struct zn_menu_bar *menu_bar;
};

struct zn_zigzag_layout *zn_zigzag_layout_create(struct wlr_output *output,
    struct wlr_renderer *renderer, struct wlr_output_damage *damage);

void zn_zigzag_layout_destroy(struct zn_zigzag_layout *self);
