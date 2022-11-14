#pragma once

#include "zen/renderer/gl-vertex-array.h"
#include "zen/renderer/rendering-unit.h"
#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_base_technique;

void znr_gl_base_technique_bind_vertex_array(struct znr_gl_base_technique* self,
    struct znr_gl_vertex_array* vertex_array);

void znr_gl_base_technique_draw_arrays(struct znr_gl_base_technique* self,
    uint32_t mode, int32_t first, uint32_t count);

struct znr_gl_base_technique* znr_gl_base_technique_create(
    struct znr_session* session, struct znr_rendering_unit* rendering_unit);

void znr_gl_base_technique_destroy(struct znr_gl_base_technique* self);

#ifdef __cplusplus
}
#endif
