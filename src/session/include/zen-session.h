#ifndef ZEN_SESSION_H
#define ZEN_SESSION_H

#include <wayland-server.h>

struct zn_session_device_changed_signal_arg {
  dev_t device;
  bool added;
};

struct zn_session {
  // data: struct zn_session_device_changed_signal_arg*
  struct wl_signal device_changed_signal;
  struct wl_signal session_signal;  // data: bool*
};

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
