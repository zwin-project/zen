#pragma once

#include "zen/renderer/dispatcher.h"
#include "zen/renderer/gl-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_vertex_array;

void znr_gl_vertex_array_enable_vertex_attrib_array(
    struct znr_gl_vertex_array *self, uint32_t index);

void znr_gl_vertex_array_disable_vertex_attrib_array(
    struct znr_gl_vertex_array *self, uint32_t index);

void znr_gl_vertex_array_vertex_attrib_pointer(struct znr_gl_vertex_array *self,
    uint32_t index, int32_t size, uint32_t type, bool normalized,
    int32_t stride, uint64_t offset, struct znr_gl_buffer *gl_buffer);

struct znr_gl_vertex_array *znr_gl_vertex_array_create(
    struct znr_dispatcher *dispatcher);

void znr_gl_vertex_array_destroy(struct znr_gl_vertex_array *self);

#ifdef __cplusplus
}
#endif
