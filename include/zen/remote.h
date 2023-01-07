#pragma once

#include <znr-remote.h>

struct zn_remote {
  struct znr_remote *znr_remote;

  struct wl_list peer_list;  // zn_peer::link

  struct {
    struct wl_signal peer_list_changed;  // (NULL)
    struct wl_signal new_session;        // (NULL)
  } events;

  struct wl_listener new_peer_listener;
};

struct zn_remote *zn_remote_create(struct wl_display *display);

void zn_remote_destroy(struct zn_remote *self);
