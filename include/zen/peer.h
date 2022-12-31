#pragma once

#include <znr-remote.h>

struct zn_peer {
  struct znr_remote_peer *znr_remote_peer;

  struct wl_listener znr_remote_peer_destroy_listener;

  struct wl_list link;  // zn_remote::peer_list
};

struct zn_peer *zn_peer_create(struct znr_remote_peer *znr_remote_peer);
