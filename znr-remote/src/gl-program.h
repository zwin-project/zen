#pragma once

#include <zen-remote/server/gl-program.h>

#include "zen/renderer/gl-program.h"

struct znr_gl_program {
  std::unique_ptr<zen::remote::server::IGlProgram> proxy;
};
