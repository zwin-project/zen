#ifndef ZEN_IMMERSIVE_BACKEND_H
#define ZEN_IMMERSIVE_BACKEND_H

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zn_immersive_backend {
  struct {
    struct wl_signal disconnected;
  } events;
};

void zn_immersive_backend_start_repaint_loop(struct zn_immersive_backend* self);

bool zn_immersive_backend_connect(struct zn_immersive_backend* self);

void zn_immersive_backend_disconnect(struct zn_immersive_backend* self);

struct zn_immersive_backend* zn_immersive_backend_create(
    struct wl_event_loop* loop);

void zn_immersive_backend_destroy(struct zn_immersive_backend* self);

#ifdef __cplusplus
}
#endif

#endif  //  ZEN_IMMERSIVE_BACKEND_H
