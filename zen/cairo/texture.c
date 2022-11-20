#include "zen/cairo/texture.h"

#include <drm_fourcc.h>

struct wlr_texture *
zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct zn_server *server)
{
  unsigned char *data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);
  return wlr_texture_from_pixels(
      server->renderer, DRM_FORMAT_ARGB8888, stride, width, height, data);
}

void
zn_cairo_draw_rounded_rectangle(
    cairo_t *cr, double width, double height, double radius)
{
  cairo_move_to(cr, radius, 0.);
  cairo_line_to(cr, width - radius, 0.);
  cairo_arc(cr, width - radius, radius, radius, -M_PI / 2, 0.);
  cairo_line_to(cr, width, height - radius);
  cairo_arc(cr, width - radius, height - radius, radius, 0., M_PI / 2);
  cairo_line_to(cr, radius, height);
  cairo_arc(cr, radius, height - radius, radius, M_PI / 2, M_PI);
  cairo_line_to(cr, 0., radius);
  cairo_arc(cr, radius, radius, radius, M_PI, 3 * M_PI / 2);
}