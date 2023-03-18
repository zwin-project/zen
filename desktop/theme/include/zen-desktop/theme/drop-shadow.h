#pragma once

#include <cairo.h>
#include <wlr/util/box.h>

struct zn_drop_shadow {
  float blur;
  float opacity;
  float radius;

  cairo_surface_t *surface;  // @nullable, @owning
};

void zn_drop_shadow_render(
    struct zn_drop_shadow *self, cairo_t *cr, struct wlr_fbox *box);

void zn_drop_shadow_init(
    struct zn_drop_shadow *self, float blur, float opacity, float radius);

void zn_drop_shadow_fini(struct zn_drop_shadow *self);
