#pragma once

#include "zgnr/gl-texture.h"

struct zgnr_gl_texture_impl {
  struct zgnr_gl_texture base;

  struct {
    // data will be unliked when comitted
    struct zn_weak_resource data;  // zgnr_shm_buffer

    uint32_t target;
    int32_t level;
    int32_t internal_format;
    uint32_t width;
    uint32_t height;
    int32_t border;
    uint32_t format;
    uint32_t type;
  } pending;
};

void zgnr_gl_texture_commit(struct zgnr_gl_texture_impl* self);

struct zgnr_gl_texture_impl* zgnr_gl_texture_create(
    struct wl_client* client, uint32_t id);
