#pragma once

#include <wayland-server-core.h>

#include "zen-common/util.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zn_virtual_object {
  void *impl_data;  // @nullable, @outlive if exists

  struct {
    struct wl_signal destroy;
  } events;
};

/// Called by impl object
struct zn_virtual_object *zn_virtual_object_create(void *impl_data);

/// Can be called by impl object
void zn_virtual_object_destroy(struct zn_virtual_object *self);

#ifdef __cplusplus
}
#endif
