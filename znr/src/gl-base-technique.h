#pragma once

#include <zen-remote/server/gl-base-technique.h>

#include "znr/gl-base-technique.h"

struct znr_gl_base_technique_impl {
  znr_gl_base_technique base;

  std::unique_ptr<zen::remote::server::IGlBaseTechnique> proxy;
};
