#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_buffer;

struct znr_gl_buffer* znr_gl_buffer_create(struct znr_session* session);

void znr_gl_buffer_destroy(struct znr_gl_buffer* self);

#ifdef __cplusplus
}
#endif
