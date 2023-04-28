#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_buffer;
struct zn_gl_vertex_array;

struct zn_gl_vertex_array_interface {
  void (*enable_vertex_attrib_array)(
      struct zn_gl_vertex_array *self, uint32_t index);
  void (*disable_vertex_attrib_array)(
      struct zn_gl_vertex_array *self, uint32_t index);
  void (*vertex_attrib_pointer)(struct zn_gl_vertex_array *self, uint32_t index,
      int32_t size, uint32_t type, bool normalized, int32_t stride,
      uint64_t offset, struct zn_gl_buffer *gl_buffer);
};

struct zn_gl_vertex_array {
  void *impl_data;                                  // @nullable, @outlive
  const struct zn_gl_vertex_array_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_vertex_array_enable_vertex_attrib_array(
    struct zn_gl_vertex_array *self, uint32_t index)
{
  self->impl->enable_vertex_attrib_array(self, index);
}

UNUSED static inline void
zn_gl_vertex_array_disable_vertex_attrib_array(
    struct zn_gl_vertex_array *self, uint32_t index)
{
  self->impl->disable_vertex_attrib_array(self, index);
}

UNUSED static inline void
zn_gl_vertex_array_vertex_attrib_pointer(struct zn_gl_vertex_array *self,
    uint32_t index, int32_t size, uint32_t type, bool normalized,
    int32_t stride, uint64_t offset, struct zn_gl_buffer *gl_buffer)
{
  self->impl->vertex_attrib_pointer(
      self, index, size, type, normalized, stride, offset, gl_buffer);
}

#ifdef __cplusplus
}
#endif
