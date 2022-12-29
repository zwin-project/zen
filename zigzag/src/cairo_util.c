#include <cairo.h>
#include <drm_fourcc.h>
#include <librsvg/rsvg.h>
#include <wlr/render/wlr_renderer.h>
#include <zen-common.h>

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
zigzag_cairo_draw_centered_text(
    cairo_t *cr, char *text, double width, double height)
{
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);
  cairo_move_to(cr, width / 2 - (extents.width / 2 + extents.x_bearing),
      height / 2 - (extents.height / 2 + extents.y_bearing));
  cairo_show_text(cr, text);
}

void
zigzag_cairo_draw_left_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding)
{
  UNUSED(width);
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);
  cairo_move_to(
      cr, padding, height / 2 - (extents.height / 2 + extents.y_bearing));
  cairo_show_text(cr, text);
}

void
zigzag_cairo_draw_right_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding)
{
  cairo_text_extents_t extents;
  cairo_text_extents(cr, text, &extents);
  cairo_move_to(cr, width - (extents.width + extents.x_bearing + padding),
      height / 2 - (extents.height / 2 + extents.y_bearing));
  cairo_show_text(cr, text);
}

void
zigzag_cairo_draw_rounded_rectangle(
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

bool
zigzag_cairo_stamp_svg_on_surface(cairo_t *cr, const char *filename, double x,
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
