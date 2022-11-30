#pragma once

#include "zgnr/program-shader.h"

struct zgnr_gl_program_impl;
struct zgnr_gl_shader_impl;

struct zgnr_program_shader_impl {
  struct zgnr_program_shader base;

  struct zgnr_gl_program_impl* program;  // nonnull

  struct wl_listener shader_destroy_listener;
};

struct zgnr_program_shader_impl* zgnr_program_shader_create(
    struct zgnr_gl_program_impl* program, struct zgnr_gl_shader_impl* shader);

void zgnr_program_shader_destroy(struct zgnr_program_shader_impl* self);
