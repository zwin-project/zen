#pragma once

#include <zen-remote/server/gl-sampler.h>

#include "zen/renderer/gl-sampler.h"

struct znr_gl_sampler {
  std::unique_ptr<zen::remote::server::IGlSampler> proxy;
};
