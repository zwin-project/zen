#ifndef ZGNR_RENDERING_UNIT_H
#define ZGNR_RENDERING_UNIT_H

#include <wayland-server-core.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_rendering_unit {
  struct zgnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

#ifdef __cplusplus
}
#endif

#endif  // ZGNR_RENDERING_UNIT_H
