#pragma once

#include <cairo.h>
#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_vr_menu;

struct zn_vr_button {
  struct zigzag_node *zigzag_node;

  cairo_surface_t *vr_icon_surface;

  struct zn_vr_menu *vr_menu;
};

struct zn_vr_button *zn_vr_button_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_vr_button_destroy(struct zn_vr_button *self);
