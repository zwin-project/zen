#pragma once
#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_renderer.h>

struct zigzag_node;
struct zigzag_color;

enum zigzag_anchor {
  ZIGZAG_ANCHOR_CENTER,
  ZIGZAG_ANCHOR_LEFT,
  ZIGZAG_ANCHOR_RIGHT,
  ZIGZAG_ANCHOR_TOP,
  ZIGZAG_ANCHOR_BOTTOM,
};

struct wlr_texture *zigzag_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer);

void zigzag_cairo_draw_node_frame(cairo_t *cr, struct zigzag_node *node,
    struct zigzag_color background_color, struct zigzag_color border_color,
    double border_width, double radius);

void zigzag_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zigzag_anchor horizontal_anchor, enum zigzag_anchor vertical_anchor);

void zigzag_cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y,
    double width, double height, double radius);

void zigzag_cairo_draw_rounded_bubble(cairo_t *cr, double x, double y,
    double width, double height, double radius, double tip_x);
