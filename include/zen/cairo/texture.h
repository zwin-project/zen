#include <cairo.h>
#include <wlr/render/wlr_texture.h>

#include "zen/server.h"

struct wlr_texture *zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct zn_server *server);

void zn_cairo_draw_rounded_rectangle(
    cairo_t *cr, double width, double height, double radius);

void zn_cairo_draw_centered_text(
    cairo_t *cr, char *text, double width, double height);

void zn_cairo_draw_left_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding);

void zn_cairo_draw_right_aligned_text(
    cairo_t *cr, char *text, double width, double height, double padding);