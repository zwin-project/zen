#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "znr/remote.h"

struct znr_gl_buffer {
  uint64_t id;
};

struct znr_gl_buffer* znr_gl_buffer_create(struct znr_remote* remote);

void znr_gl_buffer_destroy(struct znr_gl_buffer* self);

#ifdef __cplusplus
}
#endif
