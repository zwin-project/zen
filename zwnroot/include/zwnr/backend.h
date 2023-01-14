#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_backend {
  struct {
    struct wl_signal new_virtual_object;  // (struct zwnr_virtual_object *)
  } events;
};

int zwnr_backend_activate(struct zwnr_backend *self);

void zwnr_backend_deactivate(struct zwnr_backend *self);

/**
 * zwnr_backend has a activate/inactivate status.
 * Activating and deactivating zwnr_backend tell clients if the server
 * currently support an immersive environment or not. Deactivation does not
 * necessarily delete all previously created resources. There may be resources
 * that can be reused by the clients at a later activation. Initially, it's
 * inactive.
 */
struct zwnr_backend *zwnr_backend_create(struct wl_display *display);

void zwnr_backend_destroy(struct zwnr_backend *self);

#ifdef __cplusplus
}
#endif
