#pragma once

#include <zen-remote/server/peer.h>

#include <memory>

#include "znr-remote.h"

struct znr_remote_peer_impl {
  znr_remote_peer base;

  std::shared_ptr<zen::remote::server::IPeer> proxy;

  wl_list link;  // znr_remote_impl::peer_list
};

znr_remote_peer_impl* znr_remote_peer_create(
    std::shared_ptr<zen::remote::server::IPeer> proxy);

void znr_remote_peer_destroy(znr_remote_peer_impl* self);
