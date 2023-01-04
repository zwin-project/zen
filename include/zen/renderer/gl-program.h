#pragma once

#include "zen/renderer/dispatcher.h"
#include "zen/renderer/gl-shader.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_program;

void znr_gl_program_attach_shader(
    struct znr_gl_program *self, struct znr_gl_shader *shader);

void znr_gl_program_link(struct znr_gl_program *self);

struct znr_gl_program *znr_gl_program_create(struct znr_dispatcher *dispatcher);

void znr_gl_program_destroy(struct znr_gl_program *self);

#ifdef __cplusplus
}
#endif
