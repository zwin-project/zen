#pragma once

#include <wayland-server-core.h>
#include <zen-remote/server/session.h>
#include <zen-remote/signal.h>

#include "zen/renderer/session.h"

struct znr_session_impl {
  znr_session base;

  std::shared_ptr<zen::remote::server::ISession> proxy;

  std::shared_ptr<zen::remote::Signal<void()>::Connection>
      disconnect_signal_connection;
};

znr_session_impl *znr_session_create(
    std::shared_ptr<zen::remote::server::ISession> proxy);
