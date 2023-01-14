#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_peer;

struct zn_headset_dialog_item_headset {
  struct zigzag_node *zigzag_node;

  uint32_t index;        // 1-indexed
  struct zn_peer *peer;  // nonnull

  struct wl_list link;  // zn_vr_modal_item_headset_dialog::link

  struct wl_listener peer_new_session_listener;
};

struct zn_headset_dialog_item_headset *zn_headset_dialog_item_headset_create(
    struct zigzag_layout *zigzag_layout, struct zn_peer *peer, uint32_t index);

void zn_headset_dialog_item_headset_destroy(
    struct zn_headset_dialog_item_headset *self);
