#pragma once

#include <wayland-server-core.h>
#include <zgnr/virtual-object.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_space {
  struct zgnr_virtual_object *virtual_object;  // nonnull

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  void *user_data;
};

void zgnr_space_enter(struct zgnr_space *self);

void zgnr_space_leave(struct zgnr_space *self);

void zgnr_space_shutdown(struct zgnr_space *self);

#ifdef __cplusplus
}
#endif
