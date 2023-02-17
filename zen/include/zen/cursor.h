#pragma once

#include <wayland-server-core.h>

struct zn_snode;

struct zn_cursor {
  struct zn_snode *snode;  // @nonnull, @owning

  struct wlr_xcursor_manager *xcursor_manager;  // @nonnull @owning

  // TODO(@Aki-7) hotspot, enable to change cursor texture
  struct wlr_texture *xcursor_texture;  // @nullable, @owning
};
