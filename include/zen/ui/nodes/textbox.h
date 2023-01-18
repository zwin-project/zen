#pragma once

#include <wlr/render/wlr_renderer.h>

#include "zigzag/node.h"

struct zn_textbox {
  struct zigzag_node *zigzag_node;

  char *text;  // nonnull
  double font_size;
  struct zigzag_color font_color;

  struct zigzag_color bg_color;
  double frame_radius;
};

struct zn_textbox *zn_textbox_create(struct zigzag_layout *zigzag_layout,
    char *text, double font_size, struct zigzag_color font_color,
    struct zigzag_color bg_color, double frame_radius);

void zn_textbox_destroy(struct zn_textbox *self);
