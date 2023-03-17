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

void zn_color_init(
    struct zn_color *self, uint8_t r, uint8_t g, uint8_t b, uint8_t a);

struct zn_color *zn_color_create(uint8_t r, uint8_t g, uint8_t b, uint8_t a);

void zn_color_destroy(struct zn_color *self);
