#pragma once

#include "zen/gl-vertex-array.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_vertex_array *zn_gl_vertex_array_create(
    void *impl_data, const struct zn_gl_vertex_array_interface *implementation);

/// Called by impl object
void zn_gl_vertex_array_destroy(struct zn_gl_vertex_array *self);

#ifdef __cplusplus
}
#endif
