#pragma once

#include "zgnr/gl-program.h"

struct zgnr_gl_program_impl {
  struct zgnr_gl_program base;
};

struct zgnr_gl_program_impl* zgnr_gl_program_create(
    struct wl_client* client, uint32_t id);
