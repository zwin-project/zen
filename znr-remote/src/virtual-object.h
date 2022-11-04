#pragma once

#include <zen-remote/server/virtual-object.h>

#include "zen/renderer/virtual-object.h"

struct znr_virtual_object {
  std::unique_ptr<zen::remote::server::IVirtualObject> proxy;
};
