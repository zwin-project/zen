#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_vr_menu_headset_connect_button {
  struct zigzag_node *zigzag_node;

  struct zn_peer *peer;

  int idx;
};

struct zn_vr_menu_headset_connect_button *
zn_vr_menu_headset_connect_button_create(struct zigzag_layout *zigzag_layout,
    struct wlr_renderer *renderer, struct zn_peer *peer, int idx);

void zn_vr_menu_headset_connect_button_destroy(
    struct zn_vr_menu_headset_connect_button *self);
