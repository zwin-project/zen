
#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen.h"

struct zn_zigzag_board_layout {
  struct zigzag_layout *zigzag_layout;
  struct zn_board_node *left;
  struct zn_board_node *center;
  struct zn_board_node *right;

  struct zn_screen *screen;
};

struct zn_zigzag_board_layout *zn_zigzag_board_layout_create(
    struct zn_screen *screen, struct wlr_renderer *renderer);

void zn_zigzag_board_layout_destroy(struct zn_zigzag_board_layout *self);

void zn_zigzag_board_layout_notify_board_changed(
    struct zn_zigzag_board_layout *self);
