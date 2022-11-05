#pragma once

#include <zen-remote/server/rendering-unit.h>

#include "zen/renderer/rendering-unit.h"

struct znr_rendering_unit {
  std::unique_ptr<zen::remote::server::IRenderingUnit> proxy;
};
