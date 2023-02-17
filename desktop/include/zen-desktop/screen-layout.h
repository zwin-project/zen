#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_screen_container;

struct zn_screen_layout {
  struct wl_list screen_container_list;  // zn_screen_container::link
};

/// Adjust the position of the screens so that they are all adjacent to each
/// other and do not overlap, while preserving positional relationships as much
/// as possible.
/// Be idempotent.
void zn_screen_layout_reposition(struct zn_screen_layout *self);

void zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_screen_container *container);

struct zn_screen_layout *zn_screen_layout_create(void);

void zn_screen_layout_destroy(struct zn_screen_layout *self);
