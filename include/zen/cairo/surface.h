#pragma once
#include <cairo.h>
#include <wlr/render/wlr_texture.h>

struct wlr_texture *zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct wlr_renderer *renderer);
