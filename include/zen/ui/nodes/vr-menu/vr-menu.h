#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_vr_menu_item_headset;

struct zn_vr_menu {
  struct zigzag_node *zigzag_node;

  double tip_x;

  struct wl_list headset_list;  // zn_vr_menu_item_headset::link

  struct wl_listener peer_list_changed_listener;
};

struct zn_vr_menu *zn_vr_menu_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, double tip_x);

void zn_vr_menu_destroy(struct zn_vr_menu *self);
