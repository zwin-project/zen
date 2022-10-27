#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <wayland-server-core.h>

#include "znr/remote.h"

struct znr_buffer_ref_event {
  uint32_t count;
};

/**
 * The ref event notifies a changes in the number of objects using and
 * referencing this buffer inside `znr`. The reference count is only increased
 * when the buffer is used as an argument to other `znr` functions.
 * When this reference count is 1 or greater, the buffer must not be deleted,
 * the data pointer used when the buffer was created must always be available,
 * and the contents of the data pointer must not be modified.
 */
struct znr_buffer {
  struct {
    struct wl_signal ref;  // (struct znr_buffer_ref_event *)
  } events;
};

struct znr_buffer* znr_buffer_create(void* data, struct znr_remote* remote);

void znr_buffer_destroy(struct znr_buffer* self);

#ifdef __cplusplus
}
#endif
