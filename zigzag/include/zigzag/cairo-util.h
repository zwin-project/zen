#pragma once
#include <cairo.h>
#include <wayland-server-core.h>

struct zigzag_node;
struct zigzag_color;

void zigzag_cairo_draw_node_frame(cairo_t *cr, struct zigzag_node *node,
    struct zigzag_color background_color, struct zigzag_color border_color,
    double border_width, double radius);
