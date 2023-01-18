#pragma once

#include <wlr/render/wlr_renderer.h>

#include "zen/screen/output.h"
#include "zigzag/node.h"

struct zn_textbox;

struct zn_vr_modal_item_keybind_description {
  struct zigzag_node *zigzag_node;

  struct zn_textbox *press;
  struct zn_textbox *meta_key;
  struct zn_textbox *plus;
  struct zn_textbox *v_key;
  struct zn_textbox *to_exit;
};

struct zn_vr_modal_item_keybind_description *
zn_vr_modal_item_keybind_description_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_vr_modal_item_keybind_description_destroy(
    struct zn_vr_modal_item_keybind_description *self);
