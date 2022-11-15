#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_shader;

struct znr_gl_shader* znr_gl_shader_create(struct znr_session* session_base,
    const char* source, size_t length, uint32_t type);

void znr_gl_shader_destroy(struct znr_gl_shader* self);

#ifdef __cplusplus
}
#endif
