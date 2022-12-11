#pragma once

#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>

#include "zen/scene/screen.h"
#include "zen/server.h"

void zigzag_screen_setup_default(
    struct zn_screen *screen, struct zn_server *server);