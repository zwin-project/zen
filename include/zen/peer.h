#pragma once

#include <znr-remote.h>

struct zn_peer {
  struct znr_remote_peer *znr_remote_peer;
  char *host;
  bool wired;

  // nonnull when the peer has the session
  struct znr_session *session;

  struct wl_listener znr_remote_peer_destroy_listener;
  struct wl_listener znr_session_disconnected_listener;

  struct wl_list link;  // zn_remote::peer_list
};

struct zn_peer *zn_peer_create(struct znr_remote_peer *znr_remote_peer);

void zn_peer_set_session(struct zn_peer *self, struct znr_session *session);
