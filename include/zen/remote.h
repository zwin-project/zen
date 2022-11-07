#pragma once

#include <wayland-server-core.h>
#include <znr-remote.h>

struct zn_remote {
  struct znr_remote *renderer;  // nonnull, owning

  struct wl_listener new_peer_listener;
  struct wl_listener session_created_listener;
  struct wl_listener session_destroyed_listener;
};

struct zn_remote *zn_remote_create(struct wl_display *display);

void zn_remote_destroy(struct zn_remote *self);
