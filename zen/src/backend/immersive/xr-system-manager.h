#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_xr_system_manager;

struct zn_xr_system_manager_interface {
  void (*destroy)(struct zn_xr_system_manager *self);
};

struct zn_xr_system_manager {
  void *impl_data;  // @nullable, @outlive if exists
  const struct zn_xr_system_manager_interface *impl;  // @nonnull, @outlive

  struct {
    struct wl_signal new_system;  // (struct zn_xr_system *)
  } events;
};

struct zn_xr_system_manager *zn_xr_system_manager_create_remote(
    struct wl_display *display);

UNUSED static inline void
zn_xr_system_manager_destroy(struct zn_xr_system_manager *self)
{
  self->impl->destroy(self);
}

#ifdef __cplusplus
}
#endif
