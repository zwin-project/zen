#ifndef ZEN_RENDER_2D_H
#define ZEN_RENDER_2D_H

#include "zen/scene/screen.h"

void zn_render_2d_screen(struct zn_screen *screen,
    struct wlr_renderer *renderer, pixman_region32_t *damage);

#endif  //  ZEN_RENDER_2D_H
