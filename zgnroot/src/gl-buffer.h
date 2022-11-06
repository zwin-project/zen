#pragma once

#include "zgnr/gl-buffer.h"

struct zgnr_gl_buffer_impl {
  struct zgnr_gl_buffer base;
};

struct zgnr_gl_buffer_impl *zgnr_gl_buffer_create(
    struct wl_client *client, uint32_t id);
