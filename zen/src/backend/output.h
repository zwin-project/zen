#pragma once

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>

struct zn_screen;

struct zn_output {
  struct zn_screen *screen;  // @nonnull, @owning

  struct wlr_output *wlr_output;  // @nonnull, @outlive

  // @nonnull, @outlive, automatically destroyed when wlr_output is destroyed
  struct wlr_output_damage *damage;

  struct wl_listener wlr_output_destroy_listener;
  struct wl_listener damage_frame_listener;
};

struct zn_output *zn_output_create(struct wlr_output *wlr_output);
