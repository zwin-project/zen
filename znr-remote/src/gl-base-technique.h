#pragma once

#include <zen-remote/server/gl-base-technique.h>

#include "zen/renderer/gl-base-technique.h"

struct znr_gl_base_technique {
  std::unique_ptr<zen::remote::server::IGlBaseTechnique> proxy;
};
