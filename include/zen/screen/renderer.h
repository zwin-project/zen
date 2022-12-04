#pragma once

#include <pixman.h>
#include <wlr/render/wlr_renderer.h>

struct zn_output;

void zn_screen_renderer_render(struct zn_output *output,
    struct wlr_renderer *renderer, pixman_region32_t *damage);
