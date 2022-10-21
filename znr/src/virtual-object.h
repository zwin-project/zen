#pragma once

#include <zen-remote/server/virtual-object.h>

#include <memory>

#include "znr/virtual-object.h"

struct znr_virtual_object_impl {
  znr_virtual_object base;

  std::unique_ptr<zen::remote::server::IVirtualObject> proxy;
};
