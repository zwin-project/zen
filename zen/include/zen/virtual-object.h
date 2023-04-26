#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_virtual_object;

struct zn_virtual_object_interface {
  void (*commit)(struct zn_virtual_object *self);
};

struct zn_virtual_object {
  void *impl_data;                                 // @nullable, @outlive
  const struct zn_virtual_object_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal destroy;
  } events;
};

UNUSED static inline void
zn_virtual_object_commit(struct zn_virtual_object *self)
{
  self->impl->commit(self);
}

#ifdef __cplusplus
}
#endif
