#pragma once

#include <zigen-gles-v32-protocol.h>

#include "zen/renderer/gl-program.h"
#include "zen/renderer/gl-sampler.h"
#include "zen/renderer/gl-texture.h"
#include "zen/renderer/gl-vertex-array.h"
#include "zen/renderer/rendering-unit.h"
#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_base_technique;

void znr_gl_base_technique_bind_vertex_array(struct znr_gl_base_technique *self,
    struct znr_gl_vertex_array *vertex_array);

void znr_gl_base_technique_bind_program(
    struct znr_gl_base_technique *self, struct znr_gl_program *program);

void znr_gl_base_technique_bind_texture(struct znr_gl_base_technique *self,
    uint32_t binding, const char *name, struct znr_gl_texture *texture,
    uint32_t target, struct znr_gl_sampler *sampler);

/**
 * @param name is nullable
 * @param value must be larger than or equal to (32 * size * count) bits
 */
void znr_gl_base_technique_gl_uniform_vector(struct znr_gl_base_technique *self,
    uint32_t location, const char *name,
    enum zgn_gl_base_technique_uniform_variable_type type, uint32_t size,
    uint32_t count, void *value);

/**
 * @param name is nullable
 * @param value must be larger than or equal to (32 * col * row * count) bits
 */
void znr_gl_base_technique_gl_uniform_matrix(struct znr_gl_base_technique *self,
    uint32_t location, const char *name, uint32_t col, uint32_t row,
    uint32_t count, bool transpose, float *value);

void znr_gl_base_technique_draw_arrays(struct znr_gl_base_technique *self,
    uint32_t mode, int32_t first, uint32_t count);

void znr_gl_base_technique_draw_elements(struct znr_gl_base_technique *self,
    uint32_t mode, uint32_t count, uint32_t type, uint64_t offset,
    struct znr_gl_buffer *element_array_buffer);

struct znr_gl_base_technique *znr_gl_base_technique_create(
    struct znr_session *session, struct znr_rendering_unit *rendering_unit);

void znr_gl_base_technique_destroy(struct znr_gl_base_technique *self);

#ifdef __cplusplus
}
#endif
