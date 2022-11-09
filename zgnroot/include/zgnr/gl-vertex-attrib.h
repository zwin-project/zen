#pragma once

#include <wayland-server-core.h>
#include <zen-common/weak-resource.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_vertex_attrib {
  uint32_t index;
  int32_t size;
  uint32_t type;
  int32_t stride;
  uint32_t offset;
  bool normalized;
  bool enabled;
  bool enable_changed;
  bool gl_buffer_changed;
  struct zn_weak_resource gl_buffer;  // (private)
};

struct zgnr_gl_buffer* zgnr_gl_vertex_attrib_get_gl_buffer(
    struct zgnr_gl_vertex_attrib* attrib);

#ifdef __cplusplus
}
#endif
