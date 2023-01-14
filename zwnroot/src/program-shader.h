#pragma once

#include "zwnr/program-shader.h"

struct zwnr_gl_program_impl;
struct zwnr_gl_shader_impl;

struct zwnr_program_shader_impl {
  struct zwnr_program_shader base;

  struct zwnr_gl_program_impl *program;  // nonnull

  struct wl_listener shader_destroy_listener;
};

struct zwnr_program_shader_impl *zwnr_program_shader_create(
    struct zwnr_gl_program_impl *program, struct zwnr_gl_shader_impl *shader);

void zwnr_program_shader_destroy(struct zwnr_program_shader_impl *self);
