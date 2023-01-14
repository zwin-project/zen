#pragma once

#include "zwnr/gl-program.h"

struct zwnr_gl_program_impl {
  struct zwnr_gl_program base;

  struct {
    struct wl_list program_shader_list;  // zwnr_program_shader::link
    bool should_link;
    bool damaged;
  } pending;

  struct wl_resource *resource;  // nonnull
};

void zwnr_gl_program_commit(struct zwnr_gl_program_impl *self);

struct zwnr_gl_program_impl *zwnr_gl_program_create(
    struct wl_client *client, uint32_t id);
