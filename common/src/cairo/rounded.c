#include "zen-common/cairo/rounded.h"

#include <math.h>

void
zn_cairo_rounded_rect(
    cairo_t *cr, double x, double y, double width, double height, double radius)
{
  cairo_move_to(cr, x, y + radius);
  cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
  cairo_line_to(cr, x + width - radius, y);
  cairo_arc(cr, x + width - radius, y + radius, radius, 3 * M_PI / 2, 2 * M_PI);
  cairo_line_to(cr, x + width, y + height - radius);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0, M_PI / 2);
  cairo_line_to(cr, x + radius, y + height);
  cairo_arc(cr, x + radius, y + height - radius, radius, M_PI / 2, M_PI);
  cairo_close_path(cr);
}
