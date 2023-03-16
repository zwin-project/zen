#pragma once

#include <wayland-server-core.h>

struct zn_snode;

struct zn_cursor {
  struct zn_snode *snode;        // @nonnull, @owning
  struct zn_snode *image_snode;  // @nonnull, @owning

  struct wlr_xcursor_manager *xcursor_manager;  // @nonnull @owning

  struct wlr_texture *xcursor_texture;  // @nullable, @owning
  struct wlr_surface *image_surface;    // @nullable, @ref
  bool use_image_surface;

  struct wl_listener server_start_listener;
  struct wl_listener server_end_listener;
  struct wl_listener image_surface_destroy_listener;
};

bool zn_cursor_set_xcursor(struct zn_cursor *self, const char *name);

bool zn_cursor_set_xcursor_default(struct zn_cursor *self);
