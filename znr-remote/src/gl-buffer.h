#pragma once

#include <zen-remote/server/gl-buffer.h>

#include "zen/renderer/gl-buffer.h"

struct znr_gl_buffer {
  std::unique_ptr<zen::remote::server::IGlBuffer> proxy;
};
