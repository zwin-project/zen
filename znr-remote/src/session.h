#pragma once

#include <wayland-server-core.h>
#include <zen-remote/server/session.h>
#include <zen-remote/signal.h>

#include <chrono>
#include <ctime>

#include "zen/renderer/session.h"

struct znr_session_impl {
  znr_session base;

  std::shared_ptr<zen::remote::server::ISession> proxy;

  std::shared_ptr<zen::remote::Signal<void()>::Connection>
      disconnect_signal_connection;

  std::chrono::steady_clock::time_point prev_frame;
  struct wl_event_source *frame_timer_source;
};

znr_session_impl *znr_session_create(
    std::unique_ptr<zen::remote::server::ISession> proxy,
    struct wl_display *display);
