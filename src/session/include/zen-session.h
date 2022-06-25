#ifndef ZEN_SESSION_H
#define ZEN_SESSION_H

#include <wayland-server.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct zn_session {};
#pragma GCC diagnostic pop

/**
 * @return 0 on success, -1 on failure
 */
int zn_session_connect(struct zn_session* self, const char* seat_id);

/**
 * @return return NULL on failure
 */
struct zn_session* zn_session_create(struct wl_display* display);

void zn_session_destroy(struct zn_session* self);

#endif  //  ZEN_SESSION_H
