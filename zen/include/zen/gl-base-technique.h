#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_base_technique;
struct zn_gl_program;

struct zn_gl_base_technique_interface {
  void (*bind_program)(
      struct zn_gl_base_technique *self, struct zn_gl_program *gl_program);
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

#ifdef __cplusplus
}
#endif
