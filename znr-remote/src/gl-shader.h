#pragma once

#include <zen-remote/server/gl-shader.h>

#include "zen/renderer/gl-shader.h"

struct znr_gl_shader {
  std::unique_ptr<zen::remote::server::IGlShader> proxy;
};
