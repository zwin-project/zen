#pragma once

#include <wayland-server-core.h>
#include <zen-remote/server/peer-manager.h>
#include <zen-remote/server/session.h>

#include <memory>

#include "session.h"
#include "znr-remote.h"

struct znr_remote_impl {
  znr_remote base;

  wl_display *display;

  std::shared_ptr<zen::remote::server::IPeerManager> peer_manager;
  wl_list peer_list;  // znr_peer_impl::link
};
