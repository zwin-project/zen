#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "znr/buffer.h"
#include "znr/remote.h"

struct znr_gl_buffer {
  uint64_t id;
};

/**
 * @param buffer Since the contents of the buffer will be used, do not modify or
 * delete the contents until the release event fires.
 */
void znr_gl_buffer_gl_buffer_data(struct znr_gl_buffer* self,
    struct znr_buffer* buffer, uint64_t target, size_t size, uint64_t usage);

struct znr_gl_buffer* znr_gl_buffer_create(struct znr_remote* remote);

void znr_gl_buffer_destroy(struct znr_gl_buffer* self);

#ifdef __cplusplus
}
#endif
