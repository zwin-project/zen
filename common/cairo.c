#include "zen-common/cairo.h"

#include <librsvg/rsvg.h>
#include <math.h>

#include "zen-common/log.h"

void
zn_cairo_draw_rounded_rectangle(
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
zn_cairo_draw_text(cairo_t *cr, char *text, double x, double y,
    enum zn_cairo_anchor horizontal_anchor,
    enum zn_cairo_anchor vertical_anchor)
{
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);

  x -= extents.x_bearing;

  switch (horizontal_anchor) {
    case ZN_CAIRO_ANCHOR_LEFT:
      break;
    case ZN_CAIRO_ANCHOR_CENTER:
      x -= extents.width / 2;
      break;
    case ZN_CAIRO_ANCHOR_RIGHT:
      x -= extents.width;
      break;
    default:
      zn_error("invalid horizon anchor");
      return;
  }

  switch (vertical_anchor) {
    case ZN_CAIRO_ANCHOR_TOP:
      y += extents.height;
      break;
    case ZN_CAIRO_ANCHOR_CENTER:
      y += extents.height / 2;
      break;
    case ZN_CAIRO_ANCHOR_BOTTOM:
      break;
    default:
      zn_error("invalid vertical anchor");
      return;
  }

  cairo_move_to(cr, x, y);
  cairo_show_text(cr, text);
}

bool
zn_cairo_stamp_svg_on_surface(cairo_t *cr, const char *filename, double x,
    double y, double width, double height)
{
  GError *error = NULL;
  GFile *file = g_file_new_for_path(filename);
  RsvgHandle *handle = rsvg_handle_new_from_gfile_sync(
      file, RSVG_HANDLE_FLAGS_NONE, NULL, &error);
  g_object_unref(file);
  if (handle == NULL) {
    zn_error("Failed to create the svg handler: %s", error->message);
    g_error_free(error);
    return false;
  }

  RsvgRectangle viewport = {
      .x = x,
      .y = y,
      .width = width,
      .height = height,
  };

  if (!rsvg_handle_render_document(handle, cr, &viewport, &error)) {
    zn_error("Failed to render the svg: %s", error->message);
    g_error_free(error);
    return false;
  }
  g_object_unref(handle);
  return true;
}
