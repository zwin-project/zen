#pragma once

#include <cglm/types.h>
#include <wayland-server-core.h>

struct zn_screen;

struct zn_screen_container {
  vec2 position;             // layout coords
  struct zn_screen *screen;  // @nonnull, @outlive

  struct wl_list link;  // zn_screen_layout::screen_container_list

  struct wl_listener screen_resized_listener;
  struct wl_listener screen_destroy_listener;
};

struct zn_screen_container *zn_screen_container_create(
    struct zn_screen *screen);
