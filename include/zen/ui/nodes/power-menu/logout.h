#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_logout {
  struct zigzag_node *zigzag_node;
};

struct zn_logout *zn_logout_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_logout_destroy(struct zn_logout *self);
