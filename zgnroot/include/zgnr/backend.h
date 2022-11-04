#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_backend {
  struct {
    struct wl_signal new_virtual_object;  // (struct zgnr_virtual_object *)
  } events;
};

int zgnr_backend_activate(struct zgnr_backend *self);

void zgnr_backend_deactivate(struct zgnr_backend *self);

/**
 * zgnr_backend has a activate/inactivate status.
 * Activating and deactivating zgnr_backend tell clients if the server
 * currently support an immersive environment or not. Deactivation does not
 * necessarily delete all previously created resources. There may be resources
 * that can be reused by the clients at a later activation. Initially, it's
 * inactive.
 */
struct zgnr_backend *zgnr_backend_create(struct wl_display *display);

void zgnr_backend_destroy(struct zgnr_backend *self);

#ifdef __cplusplus
}
#endif
