#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_board_selector_item_switch_button;

struct zn_board_selector {
  struct zigzag_node *zigzag_node;

  struct zn_screen *screen;  // nonnull
  struct wl_list
      board_selector_item_switch_button_list;  // zn_board_selector_item_switch_button::link

  struct wl_listener board_mapped_to_screen_listener;
  struct wl_listener screen_current_board_changed_listener;
};

void zn_board_selector_update(struct zn_board_selector *self);

struct zn_board_selector *zn_board_selector_create(
    struct zigzag_layout *zigzag_layout, struct zn_screen *screen);

void zn_board_selector_destroy(struct zn_board_selector *self);
