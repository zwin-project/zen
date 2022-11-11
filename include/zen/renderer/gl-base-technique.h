#pragma once

#include "zen/renderer/rendering-unit.h"
#include "zen/renderer/session.h"

#ifdef __cplusplus
extern "C" {
#endif

struct znr_gl_base_technique;

struct znr_gl_base_technique* znr_gl_base_technique_create(
    struct znr_session* session, struct znr_rendering_unit* rendering_unit);

void znr_gl_base_technique_destroy(struct znr_gl_base_technique* self);

#ifdef __cplusplus
}
#endif
