#pragma once

#include <wlr/render/wlr_renderer.h>
#include <zigzag.h>

struct zn_clock {
  struct zigzag_node *zigzag_node;

  // TODO: Prepare a common struct for clock and power_button
  struct wl_event_source *second_timer_source;
};

struct zn_clock *zn_clock_create(
    struct zigzag_layout *zigzag_layout, struct wlr_renderer *renderer);

void zn_clock_destroy(struct zn_clock *self);
