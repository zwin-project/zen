#pragma once

#include <zen-common/weak-resource.h>

#include "zgnr/gl-buffer.h"

struct zgnr_gl_buffer_impl {
  struct zgnr_gl_buffer base;

  struct {
    // data will be unliked when comitted
    struct zn_weak_resource data;  // zgnr_shm_buffer
    uint32_t target;
    uint32_t usage;
  } pending;
};

void zgnr_gl_buffer_commit(struct zgnr_gl_buffer_impl *self);

struct zgnr_gl_buffer_impl *zgnr_gl_buffer_create(
    struct wl_client *client, uint32_t id);
