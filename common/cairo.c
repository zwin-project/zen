#include "zen-common/cairo.h"

#include <drm_fourcc.h>
#include <librsvg/rsvg.h>
#include <math.h>

#include "zen-common/log.h"

struct wlr_texture *
zn_wlr_texture_from_cairo_surface(
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
zn_cairo_draw_rounded_bubble(cairo_t *cr, double x, double y, double width,
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

void
zn_cairo_get_text_extents(char *utf8, double font_size, cairo_font_face_t *face,
    cairo_text_extents_t *extents)
{
  cairo_surface_t *surface =
      cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);

  cairo_t *cr = cairo_create(surface);

  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create cairo_t");
    goto out;
  }

  cairo_set_font_face(cr, face);
  cairo_set_font_size(cr, font_size);
  cairo_text_extents(cr, utf8, extents);
  cairo_destroy(cr);

out:
  cairo_surface_destroy(surface);
}
