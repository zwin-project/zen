#pragma once

#include <zen-remote/server/gl-vertex-array.h>

#include "zen/renderer/gl-vertex-array.h"

struct znr_gl_vertex_array {
  std::unique_ptr<zen::remote::server::IGlVertexArray> proxy;
};
