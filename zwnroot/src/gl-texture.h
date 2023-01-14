#pragma once

#include "zwnr/gl-texture.h"

struct zwnr_gl_texture_impl {
  struct zwnr_gl_texture base;

  struct {
    // data will be unliked when comitted
    struct zn_weak_resource data;  // zwnr_shm_buffer

    uint32_t target;
    int32_t level;
    int32_t internal_format;
    uint32_t width;
    uint32_t height;
    int32_t border;
    uint32_t format;
    uint32_t type;

    uint32_t generate_mipmap_target;
  } pending;
};

void zwnr_gl_texture_commit(struct zwnr_gl_texture_impl *self);

struct zwnr_gl_texture_impl *zwnr_gl_texture_create(
    struct wl_client *client, uint32_t id);
