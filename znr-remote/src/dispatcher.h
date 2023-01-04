#pragma once

#include <zen-remote/server/channel.h>

#include <chrono>
#include <ctime>

#include "zen/renderer/dispatcher.h"

struct znr_dispatcher_impl {
  znr_dispatcher base;

  std::shared_ptr<zen::remote::server::IChannel> channel;

  std::chrono::steady_clock::time_point prev_frame;
  struct wl_event_source *frame_timer_source;
};

znr_dispatcher_impl *znr_dispatcher_create(
    std::shared_ptr<zen::remote::server::ISession> &session,
    struct wl_display *display);
