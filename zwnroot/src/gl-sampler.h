#pragma once

#include "zwnr/gl-sampler.h"

struct zwnr_gl_sampler_impl {
  struct zwnr_gl_sampler base;

  struct {
    struct wl_array parameters;
  } pending;
};

void zwnr_gl_sampler_commit(struct zwnr_gl_sampler_impl *self);

struct zwnr_gl_sampler_impl *zwnr_gl_sampler_create(
    struct wl_client *client, uint32_t id);
