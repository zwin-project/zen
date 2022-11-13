#pragma once

#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_vertex_array;

struct znr_gl_vertex_array* znr_gl_vertex_array_create(
    struct znr_session* session);

void znr_gl_vertex_array_destroy(struct znr_gl_vertex_array* self);

#ifdef __cplusplus
}
#endif
