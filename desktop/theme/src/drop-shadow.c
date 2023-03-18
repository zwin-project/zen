#include "zen-desktop/theme/drop-shadow.h"

#include "zen-common/cairo.h"
#include "zen-common/log.h"
#include "zen-common/util.h"

static bool
blur_surface(cairo_surface_t *surface, int32_t blur)
{
  const int32_t kernel_size = blur * 2 + 1;
  double kernel[kernel_size];
  double kernel_sum = 0;

  int32_t width = cairo_image_surface_get_width(surface);
  int32_t height = cairo_image_surface_get_height(surface);
  int32_t stride = cairo_image_surface_get_stride(surface);
  uint8_t *data = cairo_image_surface_get_data(surface);

  uint32_t buffer_size = height * stride;
  uint8_t *buffer = malloc(buffer_size);
  if (buffer == NULL) {
    return false;
  }

  for (int i = 0; i < kernel_size; i++) {
    double f = i - blur;
    kernel[i] = (exp(-f * f / kernel_size) * 10000);
    kernel_sum += kernel[i];
  }

  for (int i = 0; i < kernel_size; i++) {
    kernel[i] /= kernel_sum;
  }

  for (long i = 0; i < height; i++) {
    uint32_t *src = (uint32_t *)(data + i * stride);
    uint32_t *dst = (uint32_t *)(buffer + i * stride);

    for (int j = 0; j < width; j++) {
      int32_t x = 0;
      int32_t y = 0;
      int32_t z = 0;
      int32_t w = 0;

      for (int k = 0; k < kernel_size; k++) {
        int index = j - blur + k;
        if (index < 0 || width <= index) {
          continue;
        }

        uint32_t p = src[index];

        x += (int32_t)((p >> 24) * kernel[k]);
        y += (int32_t)(((p >> 16) & 0xff) * kernel[k]);
        z += (int32_t)(((p >> 8) & 0xff) * kernel[k]);
        w += (int32_t)((p & 0xff) * kernel[k]);
      }

      dst[j] = (x << 24) | (y << 16) | (z << 8) | w;
    }
  }

  for (long i = 0; i < height; i++) {
    uint32_t *dst = (uint32_t *)(data + i * stride);

    for (int j = 0; j < width; j++) {
      int32_t x = 0;
      int32_t y = 0;
      int32_t z = 0;
      int32_t w = 0;

      for (int k = 0; k < kernel_size; k++) {
        long index = i - blur + k;
        if (index < 0 || height <= index) {
          continue;
        }
        uint32_t *src = (uint32_t *)(buffer + index * stride);

        uint32_t p = src[j];

        x += (int32_t)((p >> 24) * kernel[k]);
        y += (int32_t)(((p >> 16) & 0xff) * kernel[k]);
        z += (int32_t)(((p >> 8) & 0xff) * kernel[k]);
        w += (int32_t)((p & 0xff) * kernel[k]);

        dst[j] = (x << 24) | (y << 16) | (z << 8) | w;
      }
    }
  }

  free(buffer);
  cairo_surface_mark_dirty(surface);

  return true;
}

static cairo_surface_t *
zn_drop_shadow_create_surface(int32_t blur, float radius)
{
  int32_t surface_size = blur * 4;
  cairo_surface_t *surface = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, surface_size, surface_size);

  if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create a cairo surface");
    goto err_surface;
  }

  cairo_t *cr = cairo_create(surface);
  if (cairo_status(cr) != CAIRO_STATUS_SUCCESS) {
    zn_error("Failed to create a cairo context");
    goto err_context;
  }

  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_set_source_rgba(cr, 0, 0, 0, 1);
  zn_cairo_rounded_rect(
      cr, blur, blur, surface_size - 2 * blur, surface_size - 2 * blur, radius);
  cairo_fill(cr);
  cairo_surface_flush(surface);

  if (!blur_surface(surface, blur)) {
    zn_error("Failed to blur shadow surface");
    goto err_context;
  }

  cairo_destroy(cr);

  return surface;

err_context:
  cairo_destroy(cr);

err_surface:
  cairo_surface_destroy(surface);
  return NULL;
}

void
zn_drop_shadow_render(
    struct zn_drop_shadow *self, cairo_t *cr, struct wlr_fbox *box)
{
  if (self->surface == NULL) {
    self->surface =
        zn_drop_shadow_create_surface((int32_t)self->blur, self->radius);
  }

  if (self->surface == NULL) {
    zn_warn_once("Failed to create a shadow surface");
    return;
  }

  double shadow_surface_width = cairo_image_surface_get_width(self->surface);
  double shadow_surface_height = cairo_image_surface_get_width(self->surface);

  cairo_set_source_rgba(cr, 0, 0, 0, self->opacity);
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_pattern_t *pattern = cairo_pattern_create_for_surface(self->surface);
  cairo_pattern_set_filter(pattern, CAIRO_FILTER_NEAREST);

  // corners
  for (int i = 0; i < 4; i++) {
    ///
    /// 00 ------- 01
    ///  |         |
    ///  |         |
    /// 10 ------- 11
    ///

    bool is_left = i & 1;
    bool is_bottom = i >> 1;

    cairo_matrix_t matrix;
    cairo_matrix_init_translate(&matrix,
        is_left ? -(box->x + box->width - shadow_surface_width + self->blur)
                : -(box->x - self->blur),
        is_bottom ? -(box->y + box->height - shadow_surface_height + self->blur)
                  : -(box->y - self->blur));
    cairo_pattern_set_matrix(pattern, &matrix);

    double shadow_width = shadow_surface_width / 2.;
    double shadow_height = shadow_surface_height / 2.;

    if (box->width / 2 + self->blur < shadow_width) {
      shadow_width = box->width / 2 + self->blur;
    }

    if (box->height / 2 + self->blur < shadow_height) {
      shadow_height = box->height / 2 + self->blur;
    }

    cairo_reset_clip(cr);
    cairo_rectangle(cr,
        is_left ? box->x + box->width + self->blur - shadow_width
                : box->x - self->blur,
        is_bottom ? box->y + box->height + self->blur - shadow_height
                  : box->y - self->blur,
        shadow_width, shadow_height);
    cairo_clip(cr);
    cairo_mask(cr, pattern);
  }

  {  // horizontal edge
    double corner_shadow_width = shadow_surface_width / 2;
    double shadow_width = box->width + 2 * self->blur - 2 * corner_shadow_width;
    double shadow_height = self->blur;

    if (shadow_width > 0) {
      cairo_matrix_t matrix;

      /// top
      cairo_matrix_init_translate(&matrix, shadow_surface_width / 2, 0);
      cairo_matrix_scale(&matrix, 8.0 / box->width, 1);
      cairo_matrix_translate(
          &matrix, -(box->x + box->width / 2), -(box->y - self->blur));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, box->x - self->blur + corner_shadow_width,
          box->y - self->blur, shadow_width, shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);

      /// bottom
      cairo_matrix_translate(&matrix, 0,
          -(self->blur - shadow_surface_height + box->height + self->blur));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, box->x - self->blur + corner_shadow_width,
          box->y + box->height, shadow_width, shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);
    }
  }

  {  // vertical edge
    double corner_shadow_height = shadow_surface_height / 2;
    double shadow_width = self->blur;
    double shadow_height =
        box->height + 2 * self->blur - 2 * corner_shadow_height;

    if (shadow_height > 0) {
      cairo_matrix_t matrix;

      /// left
      cairo_matrix_init_translate(&matrix, 0, shadow_surface_height / 2);
      cairo_matrix_scale(&matrix, 1, 8.0 / box->height);
      cairo_matrix_translate(
          &matrix, -(box->x - self->blur), -(box->y + box->height / 2));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, box->x - self->blur,
          box->y - self->blur + corner_shadow_height, shadow_width,
          shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);

      /// right
      cairo_matrix_translate(&matrix,
          -(self->blur - shadow_surface_width + box->width + self->blur), 0);
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, box->x + box->width,
          box->y - self->blur + corner_shadow_height, shadow_width,
          shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);
    }
  }

  cairo_pattern_destroy(pattern);
  cairo_reset_clip(cr);
}

void
zn_drop_shadow_init(
    struct zn_drop_shadow *self, float blur, float opacity, float radius)
{
  self->blur = blur;
  self->opacity = opacity;
  self->radius = radius;

  self->surface = NULL;
}

void
zn_drop_shadow_fini(struct zn_drop_shadow *self)
{
  if (self->surface) {
    cairo_surface_destroy(self->surface);
  }
}
