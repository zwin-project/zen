#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_gl_virtual_object;

struct zn_gl_virtual_object_interface {
  void (*commited)(struct zn_gl_virtual_object *self);
  void (*change_visibility)(struct zn_gl_virtual_object *self, bool visible);
};

struct zn_gl_virtual_object {
  void *impl_data;  // @nullable, @outlive if exists
  const struct zn_gl_virtual_object_interface *impl;  // @nonnull, @outlive

  struct zn_xr_dispatcher *dispatcher;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

UNUSED static inline void
zn_gl_virtual_object_change_visibility(
    struct zn_gl_virtual_object *self, bool visible)
{
  self->impl->change_visibility(self, visible);
}

UNUSED static inline void
zn_gl_virtual_object_committed(struct zn_gl_virtual_object *self)
{
  self->impl->commited(self);
}

/// Called by impl object
struct zn_gl_virtual_object *zn_gl_virtual_object_create(void *impl_data,
    const struct zn_gl_virtual_object_interface *implementation,
    struct zn_xr_dispatcher *dispatcher);

/// Called by impl object
void zn_gl_virtual_object_destroy(struct zn_gl_virtual_object *self);

#ifdef __cplusplus
}
#endif
