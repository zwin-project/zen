#pragma once

#include <wayland-server-core.h>
#include <zen-common/weak-resource.h>

#include "mem-storage.h"

#ifdef __cplusplus
extern "C" {
#endif

struct zgnr_gl_texture {
  struct {
    struct wl_signal destroy;  // (NULL)
  } events;

  struct {
    struct zgnr_mem_storage* data;  // nullable

    uint32_t target;
    int32_t level;
    int32_t internal_format;
    uint32_t width;
    uint32_t height;
    int32_t border;
    uint32_t format;
    uint32_t type;

    // User may assign false to this; zgnr will only assign true to this.
    bool data_damaged;
  } current;

  void* user_data;
};

#ifdef __cplusplus
}
#endif
