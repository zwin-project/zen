#pragma once

#include <wayland-server-core.h>

#include "mem-storage.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zwnr_gl_buffer {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct zwnr_mem_storage *data;  // nullable
    uint32_t target;
    uint32_t usage;

    // User may assign false to this; zwnr will only assign true to this.
    bool data_damaged;
  } current;

  void *user_data;
};

#ifdef __cplusplus
}
#endif
