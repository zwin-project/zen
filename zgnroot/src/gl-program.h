#pragma once

#include "zgnr/gl-program.h"

struct zgnr_gl_program_impl {
  struct zgnr_gl_program base;

  struct {
    struct wl_list program_shader_list;  // zgnr_program_shader::link
    bool should_link;
  } pending;

  struct wl_resource *resource;  // nonnull
};

void zgnr_gl_program_commit(struct zgnr_gl_program_impl *self);

struct zgnr_gl_program_impl *zgnr_gl_program_create(
    struct wl_client *client, uint32_t id);
