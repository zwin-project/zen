#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_headset_dialog;

struct zn_vr_modal {
  struct zigzag_node *zigzag_node;
  struct zn_headset_dialog *headset_dialog;
};

struct zn_vr_modal *zn_vr_modal_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_vr_modal_destroy(struct zn_vr_modal *self);
