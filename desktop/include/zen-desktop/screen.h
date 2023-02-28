#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_screen;

struct zn_desktop_screen {
  vec2 position;             // layout coords
  struct zn_screen *screen;  // @nonnull, @outlive

  struct zn_snode *cursor_layer;  // @nonnull, @owning
  struct zn_snode *view_layer;    // @nonnull, @owning

  struct wl_list link;  // zn_screen_layout::desktop_screen_list

  struct wl_listener screen_resized_listener;
  struct wl_listener screen_destroy_listener;
};

void zn_desktop_screen_effective_to_layout_coords(
    struct zn_desktop_screen *self, vec2 effective, vec2 layout);

struct zn_desktop_screen *zn_desktop_screen_get(struct zn_screen *screen);

struct zn_desktop_screen *zn_desktop_screen_create(struct zn_screen *screen);
