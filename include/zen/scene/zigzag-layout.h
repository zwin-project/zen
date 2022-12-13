#pragma once

#include <cairo.h>
#include <wayland-server-core.h>
#include <wlr/render/wlr_texture.h>
#include <zigzag.h>

#include "zen/output.h"
#include "zen/scene/screen.h"
#include "zen/server.h"

struct zen_zigzag_layout_state {
  struct zn_output *output;
};

void zen_zigzag_layout_on_damage(struct zigzag_node *node);

void zigzag_layout_setup_default(
    struct zigzag_layout *node_layout, struct zn_server *server);
