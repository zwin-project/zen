#pragma once

#include "zen/scene/screen.h"

void zn_screen_renderer_render(struct zn_screen *screen,
    struct wlr_renderer *renderer, pixman_region32_t *damage);
