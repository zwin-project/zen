#include "zen-desktop/theme.h"

#include <math.h>
#include <stdint.h>

#include "zen-common/cairo/rounded.h"
#include "zen-common/log.h"
#include "zen-common/util.h"
#include "zen-desktop/shell.h"

void
zn_theme_cairo_drop_shadow(
    struct zn_theme *self, cairo_t *cr, struct wlr_fbox *box)
{
  int32_t x = (int32_t)box->x;
  int32_t y = (int32_t)box->y;
  int32_t width = (int32_t)box->width;
  int32_t height = (int32_t)box->height;

  int shadow_surface_width =
      cairo_image_surface_get_width(self->shadow_surface);
  int shadow_surface_height =
      cairo_image_surface_get_width(self->shadow_surface);

  cairo_set_source_rgba(cr, 0, 0, 0, 0.45);
  cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
  cairo_pattern_t *pattern =
      cairo_pattern_create_for_surface(self->shadow_surface);
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
        is_left ? -(x + width - shadow_surface_width + self->shadow_blur)
                : -(x - self->shadow_blur),
        is_bottom ? -(y + height - shadow_surface_height + self->shadow_blur)
                  : -(y - self->shadow_blur));
    cairo_pattern_set_matrix(pattern, &matrix);

    int32_t shadow_width = shadow_surface_width / 2;
    int32_t shadow_height = shadow_surface_height / 2;

    if (width / 2 + self->shadow_blur < shadow_width) {
      shadow_width = (width + (is_left ? 1 : 0)) / 2 + self->shadow_blur;
    }

    if (height / 2 + self->shadow_blur < shadow_height) {
      shadow_height = (height + (is_bottom ? 1 : 0)) / 2 + self->shadow_blur;
    }

    cairo_reset_clip(cr);
    cairo_rectangle(cr,
        is_left ? x + width + self->shadow_blur - shadow_width
                : x - self->shadow_blur,
        is_bottom ? y + height + self->shadow_blur - shadow_height
                  : y - self->shadow_blur,
        shadow_width, shadow_height);
    cairo_clip(cr);
    cairo_mask(cr, pattern);
  }

  {  // horizontal edge
    int32_t corner_shadow_width = shadow_surface_width / 2;
    int32_t shadow_width =
        width + 2 * self->shadow_blur - 2 * corner_shadow_width;
    int32_t shadow_height = self->shadow_blur;

    if (shadow_width > 0) {
      cairo_matrix_t matrix;

      /// top
      // NOLINTNEXTLINE(bugprone-integer-division)
      cairo_matrix_init_translate(&matrix, shadow_surface_width / 2, 0);
      cairo_matrix_scale(&matrix, 8.0 / width, 1);
      cairo_matrix_translate(
          // NOLINTNEXTLINE(bugprone-integer-division)
          &matrix, -(x + width / 2), -(y - self->shadow_blur));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, x - self->shadow_blur + corner_shadow_width,
          y - self->shadow_blur, shadow_width, shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);

      /// bottom
      cairo_matrix_translate(&matrix, 0,
          -(self->shadow_blur - shadow_surface_height + height +
              self->shadow_blur));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, x - self->shadow_blur + corner_shadow_width,
          y + height, shadow_width, shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);
    }
  }

  {  // vertical edge
    int32_t corner_shadow_height = shadow_surface_height / 2;
    int32_t shadow_width = self->shadow_blur;
    int32_t shadow_height =
        height + 2 * self->shadow_blur - 2 * corner_shadow_height;

    if (shadow_height > 0) {
      cairo_matrix_t matrix;

      /// left
      // NOLINTNEXTLINE(bugprone-integer-division)
      cairo_matrix_init_translate(&matrix, 0, shadow_surface_height / 2);
      cairo_matrix_scale(&matrix, 1, 8.0 / height);
      cairo_matrix_translate(
          // NOLINTNEXTLINE(bugprone-integer-division)
          &matrix, -(x - self->shadow_blur), -(y + height / 2));
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, x - self->shadow_blur,
          y - self->shadow_blur + corner_shadow_height, shadow_width,
          shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);

      /// right
      cairo_matrix_translate(&matrix,
          -(self->shadow_blur - shadow_surface_width + width +
              self->shadow_blur),
          0);
      cairo_pattern_set_matrix(pattern, &matrix);

      cairo_reset_clip(cr);
      cairo_rectangle(cr, x + width,
          y - self->shadow_blur + corner_shadow_height, shadow_width,
          shadow_height);
      cairo_clip(cr);
      cairo_mask(cr, pattern);
    }
  }

  cairo_pattern_destroy(pattern);
  cairo_reset_clip(cr);
}

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
zn_theme_create_shadow_surface(float radius, int32_t blur)
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

struct zn_theme *
zn_theme_get(void)
{
  struct zn_desktop_shell *shell = zn_desktop_shell_get_singleton();
  return shell->theme;
}

struct zn_theme *
zn_theme_create(void)
{
  struct zn_theme *self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->frame_radius = 8.F;
  self->shadow_blur = 35;
  self->header_bar_height = 30;

  self->shadow_surface =
      zn_theme_create_shadow_surface(self->frame_radius, self->shadow_blur);
  if (self->shadow_surface == NULL) {
    zn_error("Failed to create a shadow surface");
    goto err_free;
  }

  return self;

err_free:
  free(self);

err:
  return NULL;
}

void
zn_theme_destroy(struct zn_theme *self)
{
  cairo_surface_destroy(self->shadow_surface);
  free(self);
}
