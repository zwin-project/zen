#include "zigzag/cairo-util.h"

#include <cairo.h>
#include <zen-common.h>

#include "zigzag/node.h"

void
zigzag_cairo_draw_node_frame(cairo_t *cr, struct zigzag_node *node,
    struct zigzag_color background_color, struct zigzag_color border_color,
    double border_width, double radius)
{
  cairo_save(cr);

  zn_cairo_draw_rounded_rectangle(
      cr, 0, 0, node->frame.width, node->frame.height, radius);

  cairo_set_source_rgba(cr, background_color.r, background_color.g,
      background_color.b, background_color.a);
  cairo_fill_preserve(cr);

  cairo_set_line_width(cr, border_width);
  cairo_set_source_rgba(
      cr, border_color.r, border_color.g, border_color.b, border_color.a);
  cairo_stroke(cr);

  cairo_restore(cr);
}
