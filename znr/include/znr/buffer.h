#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server-core.h>

#include "znr/remote.h"

struct znr_buffer {
  struct {
    struct wl_signal release;  // (NULL)
  } events;
};

struct znr_buffer* znr_buffer_create(void* data, struct znr_remote* remote);

void znr_buffer_destroy(struct znr_buffer* self);

#ifdef __cplusplus
}
#endif
