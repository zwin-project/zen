#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_shell {
  struct {
    struct wl_signal new_bounded;    // (struct zgnr_bounded)
    struct wl_signal new_expansive;  // (struct zgnr_expansive)
  } events;
};

struct zgnr_shell *zgnr_shell_create(struct wl_display *display);

void zgnr_shell_destroy(struct zgnr_shell *self);

#ifdef __cplusplus
}
#endif
