#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_lease_buffer;
struct zn_gl_buffer;

struct zn_gl_buffer_interface {
  /// @param lease_buffer will be released by the callee
  void (*data)(struct zn_gl_buffer *self, uint32_t target,
      struct zn_lease_buffer *lease_buffer, uint32_t usage);
};

struct zn_gl_buffer {
  void *impl_data;                            // @nullable, @outlive
  const struct zn_gl_buffer_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

/// @param lease_buffer will be released by the callee
UNUSED static inline void
zn_gl_buffer_data(struct zn_gl_buffer *self, uint32_t target,
    struct zn_lease_buffer *lease_buffer, uint32_t usage)
{
  self->impl->data(self, target, lease_buffer, usage);
}

#ifdef __cplusplus
}
#endif
