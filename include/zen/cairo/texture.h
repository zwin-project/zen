#include <cairo.h>
#include <wlr/render/wlr_texture.h>

#include "zen/server.h"

struct wlr_texture *zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct zn_server *server);

void zn_cairo_draw_rounded_rectangle(
    cairo_t *cr, double width, double height, double radius);