#include "zigzag/cairo-util.h"

#include <cairo.h>
#include <drm_fourcc.h>
#include <librsvg/rsvg.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>

#include "zigzag/node.h"

struct wlr_texture *
zigzag_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer)
{
  unsigned char *data = cairo_image_surface_get_data(surface);
  int stride = cairo_image_surface_get_stride(surface);
  int width = cairo_image_surface_get_width(surface);
  int height = cairo_image_surface_get_height(surface);
  return wlr_texture_from_pixels(
      renderer, DRM_FORMAT_ARGB8888, stride, width, height, data);
}

void
zigzag_cairo_draw_node_frame(cairo_t *cr, struct zigzag_node *node,
    struct zigzag_color background_color, struct zigzag_color border_color,
    double border_width, double radius)
{
  cairo_save(cr);

  zigzag_cairo_draw_rounded_rectangle(
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

void
zigzag_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zigzag_anchor horizontal_anchor, enum zigzag_anchor vertical_anchor)
{
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);

  x -= extents.x_bearing;

  switch (horizontal_anchor) {
    case ZIGZAG_ANCHOR_LEFT:
      break;
    case ZIGZAG_ANCHOR_CENTER:
      x -= extents.width / 2;
      break;
    case ZIGZAG_ANCHOR_RIGHT:
      x -= extents.width;
      break;
    default:
      zn_error("invalid horizon anchor");
      return;
  }

  switch (vertical_anchor) {
    case ZIGZAG_ANCHOR_TOP:
      y += extents.height;
      break;
    case ZIGZAG_ANCHOR_CENTER:
      y += extents.height / 2;
      break;
    case ZIGZAG_ANCHOR_BOTTOM:
      break;
    default:
      zn_error("invalid vertical anchor");
      return;
  }

  cairo_move_to(cr, x, y);
  cairo_show_text(cr, text);
}

void
zigzag_cairo_draw_rounded_rectangle(
    cairo_t *cr, double x, double y, double width, double height, double radius)
{
  cairo_move_to(cr, x + radius, y);
  cairo_line_to(cr, x + width - radius, y);
  cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI / 2, 0.);
  cairo_line_to(cr, x + width, y + height - radius);
  cairo_arc(cr, x + width - radius, y + height - radius, radius, 0., M_PI / 2);
  cairo_line_to(cr, x + radius, y + height);
  cairo_arc(cr, x + radius, y + height - radius, radius, M_PI / 2, M_PI);
  cairo_line_to(cr, x, y + radius);
  cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
}

void
zigzag_cairo_draw_rounded_bubble(cairo_t *cr, double x, double y, double width,
    double height, double radius, double tip_x)
{
  const double tip_height = 5;
  const double tip_width = 10;
  const double rectangle_height = height - tip_height;
  cairo_move_to(cr, x + radius, y);
  cairo_line_to(cr, x + width - radius, y);
  cairo_arc(cr, x + width - radius, y + radius, radius, -M_PI / 2, 0.);
  cairo_line_to(cr, x + width, y + rectangle_height - radius);
  cairo_arc(cr, x + width - radius, y + rectangle_height - radius, radius, 0.,
      M_PI / 2);

  cairo_line_to(cr, tip_x + tip_width / 2, y + rectangle_height);
  cairo_line_to(cr, tip_x, y + height);
  cairo_line_to(cr, tip_x - tip_width / 2, y + rectangle_height);

  cairo_line_to(cr, x + radius, y + rectangle_height);
  cairo_arc(
      cr, x + radius, y + rectangle_height - radius, radius, M_PI / 2, M_PI);
  cairo_line_to(cr, x, y + radius);
  cairo_arc(cr, x + radius, y + radius, radius, M_PI, 3 * M_PI / 2);
}
