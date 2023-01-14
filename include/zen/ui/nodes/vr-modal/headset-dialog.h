#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_textbox;

struct zn_vr_modal_item_headset_dialog {
  struct zigzag_node *zigzag_node;

  struct wl_list headset_list;  // zn_headset_dialog_item_headset::link

  struct zn_textbox *headset_text;
  struct zn_textbox *no_headset;  // nullable

  struct wl_listener peer_list_changed_listener;
};

struct zn_vr_modal_item_headset_dialog *zn_vr_modal_item_headset_dialog_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_vr_modal_item_headset_dialog_destroy(
    struct zn_vr_modal_item_headset_dialog *self);
