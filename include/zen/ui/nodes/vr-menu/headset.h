#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_vr_menu_headset_connect_button;

struct zn_vr_menu_item_headset {
  struct zigzag_node *zigzag_node;

  struct wl_list link;  // For zn_vr_menu::headset_list;

  struct zn_peer *peer;

  cairo_surface_t *vr_icon_surface;

  struct zn_vr_menu_headset_connect_button *connect_button;

  int idx;
};

struct zn_vr_menu_item_headset *zn_vr_menu_item_headset_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer,
    struct zn_peer *peer, int idx);

void zn_vr_menu_item_headset_destroy(struct zn_vr_menu_item_headset *self);
