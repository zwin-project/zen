#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

#include "zen/screen/output.h"

struct zn_headset_status {
  struct zigzag_node *zigzag_node;

  struct wl_listener peer_list_changed_listener;
};

struct zn_headset_status *zn_headset_status_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_headset_status_destroy(struct zn_headset_status *self);
