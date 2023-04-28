#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_base_technique;
struct zn_gl_buffer;
struct zn_gl_program;
struct zn_gl_vertex_array;

struct zn_gl_base_technique_interface {
  void (*bind_program)(
      struct zn_gl_base_technique *self, struct zn_gl_program *gl_program);
  void (*bind_vertex_array)(struct zn_gl_base_technique *self,
      struct zn_gl_vertex_array *gl_vertex_array);
  void (*uniform_vector)(struct zn_gl_base_technique *self, uint32_t location,
      const char *name, uint32_t type, uint32_t size, uint32_t count,
      void *value);
  void (*uniform_matrix)(struct zn_gl_base_technique *self, uint32_t location,
      const char *name, uint32_t col, uint32_t row, uint32_t count,
      bool transpose, void *value);
  void (*draw_arrays)(struct zn_gl_base_technique *self, uint32_t mode,
      int32_t first, uint32_t count);
  void (*draw_elements)(struct zn_gl_base_technique *self, uint32_t mode,
      uint32_t count, uint32_t type, uint64_t offset,
      struct zn_gl_buffer *gl_buffer);
};

struct zn_gl_base_technique {
  void *impl_data;                                    // @nullable, @outlive
  const struct zn_gl_base_technique_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_base_technique_bind_program(
    struct zn_gl_base_technique *self, struct zn_gl_program *gl_program)
{
  self->impl->bind_program(self, gl_program);
}

UNUSED static inline void
zn_gl_base_technique_bind_vertex_array(struct zn_gl_base_technique *self,
    struct zn_gl_vertex_array *gl_vertex_array)
{
  self->impl->bind_vertex_array(self, gl_vertex_array);
}

UNUSED static inline void
zn_gl_base_technique_uniform_vector(struct zn_gl_base_technique *self,
    uint32_t location, const char *name, uint32_t type, uint32_t size,
    uint32_t count, void *value)
{
  self->impl->uniform_vector(self, location, name, type, size, count, value);
}

UNUSED static inline void
zn_gl_base_technique_uniform_matrix(struct zn_gl_base_technique *self,
    uint32_t location, const char *name, uint32_t col, uint32_t row,
    uint32_t count, bool transpose, void *value)
{
  self->impl->uniform_matrix(
      self, location, name, col, row, count, transpose, value);
}

UNUSED static inline void
zn_gl_base_technique_draw_arrays(struct zn_gl_base_technique *self,
    uint32_t mode, int32_t first, uint32_t count)
{
  self->impl->draw_arrays(self, mode, first, count);
}

UNUSED static inline void
zn_gl_base_technique_draw_elements(struct zn_gl_base_technique *self,
    uint32_t mode, uint32_t count, uint32_t type, uint64_t offset,
    struct zn_gl_buffer *gl_buffer)
{
  self->impl->draw_elements(self, mode, count, type, offset, gl_buffer);
}

#ifdef __cplusplus
}
#endif
