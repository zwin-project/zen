#pragma once

#include "weak-resource.h"
#include "zgnr/gl-buffer.h"

struct zgnr_gl_buffer_impl {
  struct zgnr_gl_buffer base;

  struct {
    struct zgnr_weak_resource data;  // zgnr_shm_buffer
    uint32_t target;
    uint32_t usage;
  } pending;
};

struct zgnr_gl_buffer_impl *zgnr_gl_buffer_create(
    struct wl_client *client, uint32_t id);
