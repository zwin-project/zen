#pragma once

#include "weak-resource.h"
#include "zgnr/gl-vertex-attrib.h"

struct zgnr_gl_vertex_attrib {
  struct zgnr_gl_vertex_attrib_info info;
  struct zgnr_weak_resource gl_buffer;
  bool enable_changed;
  bool data_changed;
};
