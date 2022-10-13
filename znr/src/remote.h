#pragma once

#include <zen-remote/remote.h>

#include <memory>

#include "znr/remote.h"

struct znr_remote_impl {
  znr_remote base;

  std::shared_ptr<zen::remote::IRemote> proxy;
};
