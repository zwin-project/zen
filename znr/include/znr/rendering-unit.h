#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "znr/remote.h"

struct znr_rendering_unit {
  uint64_t id;
};

struct znr_rendering_unit* znr_rendering_unit_create(
    struct znr_remote* remote, uint64_t virtual_object_id);

void znr_rendering_unit_gl_enable_vertex_attrib_array(
    struct znr_rendering_unit* self, uint32_t index);

void znr_rendering_unit_gl_disable_vertex_attrib_array(
    struct znr_rendering_unit* self, uint32_t index);

void znr_rendering_unit_gl_vertex_attrib_pointer(
    struct znr_rendering_unit* self, uint32_t index, uint64_t gl_buffer_id,
    int32_t size, uint64_t type, bool normalized, int32_t stride,
    uint64_t offset);

void znr_rendering_unit_destroy(struct znr_rendering_unit* self);

#ifdef __cplusplus
}
#endif
