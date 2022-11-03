#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "znr/remote.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
struct znr_gl_base_technique {};
#pragma GCC diagnostic pop

struct znr_gl_base_technique* znr_gl_base_technique_create(
    struct znr_remote* remote, uint64_t rendering_unit_id);

void znr_gl_base_technique_gl_draw_arrays(struct znr_gl_base_technique* parent,
    uint32_t mode, int32_t first, uint32_t count);

void znr_gl_base_technique_destroy(struct znr_gl_base_technique* self);

#ifdef __cplusplus
}
#endif
