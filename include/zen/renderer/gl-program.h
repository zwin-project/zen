#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_program;

struct znr_gl_program* znr_gl_program_create(struct znr_session* session);

void znr_gl_program_destroy(struct znr_gl_program* self);

#ifdef __cplusplus
}
#endif
