#pragma once

#include <zen-common/weak-resource.h>

#include "zwnr/gl-buffer.h"

struct zwnr_gl_buffer_impl {
  struct zwnr_gl_buffer base;

  struct {
    // data will be unliked when comitted
    struct zn_weak_resource data;  // zwnr_shm_buffer
    uint32_t target;
    uint32_t usage;
  } pending;
};

void zwnr_gl_buffer_commit(struct zwnr_gl_buffer_impl *self);

struct zwnr_gl_buffer_impl *zwnr_gl_buffer_create(
    struct wl_client *client, uint32_t id);
