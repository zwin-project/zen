#pragma once

#include <zen-remote/server/session.h>

#include "zen/renderer/session.h"

struct znr_session_impl {
  znr_session base;

  std::shared_ptr<zen::remote::server::ISession> proxy;
};

znr_session_impl* znr_session_create(
    std::unique_ptr<zen::remote::server::ISession> proxy);

void znr_session_destroy(znr_session_impl* self);
