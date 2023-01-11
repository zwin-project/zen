#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_board;
struct zn_board_selector;

struct zn_board_selector_item_switch_button {
  struct zigzag_node *zigzag_node;
  struct zn_board_selector *parent;
  uint32_t index;  // 1-indexed

  struct zn_board *board;
  struct wl_list
      link;  // zn_board_selector::board_selector_item_switch_button_list

  struct wl_listener board_destroy_listener;
};

void zn_board_selector_item_switch_button_update(
    struct zn_board_selector_item_switch_button *self,
    struct wlr_renderer *renderer);

struct zn_board_selector_item_switch_button *
zn_board_selector_item_switch_button_create(struct zigzag_layout *zigzag_layout,
    struct zn_board *board, struct zn_board_selector *parent, uint32_t index);

void zn_board_selector_item_switch_button_destroy(
    struct zn_board_selector_item_switch_button *self);
