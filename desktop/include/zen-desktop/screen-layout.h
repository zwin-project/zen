#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_desktop_screen;

struct zn_screen_layout {
  struct wl_list desktop_screen_list;  // zn_desktop_screen::link
};

/// @param desktop_screen : nonnull
/// @param position : effective coord
/// @param position_out : effective coord
/// position and position_out can be the same one
/// desktop_screen and desktop_screen_out can be the same one
void zn_screen_layout_get_closest_position(struct zn_screen_layout *self,
    struct zn_desktop_screen *desktop_screen, vec2 position,
    struct zn_desktop_screen **desktop_screen_out, vec2 position_out);

/// @return value is nullable
struct zn_desktop_screen *zn_screen_layout_get_main_screen(
    struct zn_screen_layout *self);

/// Adjust the position of the screens so that they are all adjacent to each
/// other and do not overlap, while preserving positional relationships as much
/// as possible.
/// Be idempotent.
void zn_screen_layout_reposition(struct zn_screen_layout *self);

void zn_screen_layout_add(
    struct zn_screen_layout *self, struct zn_desktop_screen *desktop_screen);

struct zn_screen_layout *zn_screen_layout_create(void);

void zn_screen_layout_destroy(struct zn_screen_layout *self);
