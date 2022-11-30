#pragma once

#include "zen/renderer/gl-shader.h"
#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_program;

void znr_gl_program_attach_shader(
    struct znr_gl_program* self, struct znr_gl_shader* shader);

void znr_gl_program_link(struct znr_gl_program* self);

struct znr_gl_program* znr_gl_program_create(struct znr_session* session);

void znr_gl_program_destroy(struct znr_gl_program* self);

#ifdef __cplusplus
}
#endif
