#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server-core.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct znr_remote {};
#pragma GCC diagnostic pop

void znr_remote_start(struct znr_remote* self);

void znr_remote_stop(struct znr_remote* self);

struct znr_remote* znr_remote_create(struct wl_display* display);

void znr_remote_destroy(struct znr_remote* self);

#ifdef __cplusplus
}
#endif
