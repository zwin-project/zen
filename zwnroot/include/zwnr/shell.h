#pragma once

#include <wayland-server-core.h>

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_shell {
  struct {
    struct wl_signal new_bounded;    // (struct zwnr_bounded)
    struct wl_signal new_expansive;  // (struct zwnr_expansive)
  } events;
};

struct zwnr_shell *zwnr_shell_create(struct wl_display *display);

void zwnr_shell_destroy(struct zwnr_shell *self);

#ifdef __cplusplus
}
#endif
