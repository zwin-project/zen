#pragma once

#include "zen/gl-shader.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_shader *zn_gl_shader_create(void *impl_data);

/// Called by impl object
void zn_gl_shader_destroy(struct zn_gl_shader *self);

#ifdef __cplusplus
}
#endif
