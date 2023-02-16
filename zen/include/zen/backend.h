#pragma once

#include <wayland-server-core.h>

struct zn_backend {
  struct wl_display *display;  // @nonnull, @outlive

  struct wlr_backend *wlr_backend;      // @nonnull, @owning
  struct wlr_renderer *wlr_renderer;    // @nonnull, @owning
  struct wlr_allocator *wlr_allocator;  // @nonnull, @owning

  struct wl_list input_device_list;  // zn_input_device_base::link

  struct {
    struct wl_signal new_screen;  // (struct zn_screen *)
    struct wl_signal destroy;     // (NULL)
  } events;

  struct wl_listener new_input_listener;
  struct wl_listener new_output_listener;
};
