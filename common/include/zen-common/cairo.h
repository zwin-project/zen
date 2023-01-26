#pragma once

#include <cairo.h>
#include <stdbool.h>
#include <wlr/render/wlr_texture.h>

enum zn_cairo_anchor {
  ZN_CAIRO_ANCHOR_CENTER,
  ZN_CAIRO_ANCHOR_LEFT,
  ZN_CAIRO_ANCHOR_RIGHT,
  ZN_CAIRO_ANCHOR_TOP,
  ZN_CAIRO_ANCHOR_BOTTOM,
};

struct wlr_texture *zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer);

void zn_cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y,
    double width, double height, double radius);

void zn_cairo_draw_rounded_bubble(cairo_t *cr, double x, double y, double width,
    double height, double radius, double tip_x);

void zn_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zn_cairo_anchor horizontal_anchor,
    enum zn_cairo_anchor vertical_anchor);

bool zn_cairo_stamp_svg_on_surface(cairo_t *cr, const char *filename, double x,
    double y, double width, double height);

void zn_cairo_get_text_extents(char *utf8, double font_size,
    cairo_font_face_t *face, cairo_text_extents_t *extends);
