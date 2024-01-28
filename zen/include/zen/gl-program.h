#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_program;
struct zn_gl_shader;

struct zn_gl_program_interface {
  void (*attach_shader)(
      struct zn_gl_program *self, struct zn_gl_shader *gl_shader);
  void (*link)(struct zn_gl_program *self);
};

struct zn_gl_program {
  void *impl_data;                             // @nullable, @outlive
  const struct zn_gl_program_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_program_attach_shader(
    struct zn_gl_program *self, struct zn_gl_shader *gl_shader)
{
  self->impl->attach_shader(self, gl_shader);
}

UNUSED static inline void
zn_gl_program_link(struct zn_gl_program *self)
{
  self->impl->link(self);
}

#ifdef __cplusplus
}
#endif
