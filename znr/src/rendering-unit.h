#pragma once

#include <memory.h>
#include <zen-remote/server/rendering-unit.h>

#include "znr/rendering-unit.h"

struct znr_rendering_unit_impl {
  znr_rendering_unit base;

  std::unique_ptr<zen::remote::server::IRenderingUnit> proxy;
};
