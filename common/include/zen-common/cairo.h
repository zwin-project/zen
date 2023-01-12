#pragma once

#include <cairo.h>

void zn_cairo_draw_rounded_rectangle(cairo_t *cr, double x, double y,
    double width, double height, double radius);
