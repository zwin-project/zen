#pragma once

#include "zgnr/gl-sampler.h"

struct zgnr_gl_sampler_impl {
  struct zgnr_gl_sampler base;

  struct {
    struct wl_array parameters;
  } pending;
};

void zgnr_gl_sampler_commit(struct zgnr_gl_sampler_impl *self);

struct zgnr_gl_sampler_impl *zgnr_gl_sampler_create(
    struct wl_client *client, uint32_t id);
