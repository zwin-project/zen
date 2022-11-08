#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_vertex_attrib;

struct zgnr_gl_vertex_attrib_info* zgnr_gl_vertex_attrib_get_info(
    struct zgnr_gl_vertex_attrib* attrib);

struct zgnr_gl_vertex_attrib_info {
  uint32_t index;
  int32_t size;
  uint32_t type;
  int32_t stride;
  uint32_t offset;
  bool normalized;
  bool enabled;
};

#ifdef __cplusplus
}
#endif
