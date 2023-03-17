#pragma once

#include <cairo.h>
#include <wlr/util/box.h>

struct zn_theme {
  float frame_radius;
  int32_t shadow_blur;
  int32_t header_bar_height;

  cairo_surface_t *shadow_surface;  // @nonnull, @owning
};

/// @param box is the content area, the shadow is attached to its surroundings
void zn_theme_cairo_drop_shadow(
    struct zn_theme *self, cairo_t *cr, struct wlr_fbox *box);

struct zn_theme *zn_theme_get(void);

struct zn_theme *zn_theme_create(void);

void zn_theme_destroy(struct zn_theme *self);
