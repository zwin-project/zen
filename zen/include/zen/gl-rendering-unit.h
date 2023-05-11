#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_rendering_unit;

struct zn_gl_rendering_unit_interface {
  void (*change_visibility)(struct zn_gl_rendering_unit *self, bool visible);
};

struct zn_gl_rendering_unit {
  void *impl_data;                                    // @nullable, @outlive
  const struct zn_gl_rendering_unit_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_rendering_unit_change_visibility(
    struct zn_gl_rendering_unit *self, bool visible)
{
  self->impl->change_visibility(self, visible);
}

#ifdef __cplusplus
}
#endif
