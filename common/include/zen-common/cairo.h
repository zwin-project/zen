#pragma once

#include <cairo.h>

enum zn_cairo_anchor {
  ZN_CAIRO_ANCHOR_CENTER,
  ZN_CAIRO_ANCHOR_LEFT,
  ZN_CAIRO_ANCHOR_RIGHT,
  ZN_CAIRO_ANCHOR_TOP,
  ZN_CAIRO_ANCHOR_BOTTOM,
};

void zn_cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y,
    double width, double height, double radius);

void zn_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zn_cairo_anchor horizontal_anchor,
    enum zn_cairo_anchor vertical_anchor);
