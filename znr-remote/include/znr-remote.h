#pragma once

#include <wayland-server-core.h>
#include <zen/renderer/dispatcher.h>
#include <zen/renderer/session.h>

#ifdef __cplusplus
extern "C" {
#endif

struct znr_remote_peer {
  char *host;  // null-terminated ip address string
  bool wired;

  struct {
    struct wl_signal destroy;  // (NULL)
  } events;
};

struct znr_remote {
  struct {
    struct wl_signal new_peer;  // (struct znr_remote_peer *)
  } events;
};

struct znr_dispatcher *znr_remote_create_dispatcher(
    struct znr_remote *self, struct znr_session *session);

struct znr_session *znr_remote_create_session(
    struct znr_remote *self, struct znr_remote_peer *peer);

struct znr_remote *znr_remote_create(struct wl_display *display);

void znr_remote_destroy(struct znr_remote *self);

#ifdef __cplusplus
}
#endif
