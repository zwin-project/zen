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