#include <cairo.h>
#include <wlr/render/wlr_texture.h>

#include "zen/server.h"

struct wlr_texture *zn_wlr_texture_from_cairo_surface(
    cairo_surface_t *surface, struct zn_server *server);