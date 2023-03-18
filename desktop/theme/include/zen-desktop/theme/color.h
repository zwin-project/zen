#pragma once

#include <cairo.h>
#include <cglm/types.h>
#include <stdint.h>

struct zn_color {
  vec4 rgba;
};

float zn_color_get_lightness(struct zn_color *self);

void zn_color_set_lightness(struct zn_color *self, float lightness);

void zn_color_set_cairo_source(struct zn_color *self, cairo_t *cr);

void zn_color_init(struct zn_color *self, uint32_t rgba);

void zn_color_fini(struct zn_color *self);
