#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen.h"
#include "zen/ui/nodes/menu-bar.h"

struct zn_zigzag_layout {
  struct zigzag_layout *zigzag_layout;

  struct zn_screen *screen;
  struct zn_menu_bar *menu_bar;
};

struct zn_zigzag_layout *zn_zigzag_layout_create(
    struct zn_screen *screen, struct wlr_renderer *renderer);

void zn_zigzag_layout_destroy(struct zn_zigzag_layout *self);
