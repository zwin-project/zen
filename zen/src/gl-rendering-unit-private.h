#pragma once

#include "zen/gl-rendering-unit.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_rendering_unit *zn_gl_rendering_unit_create(void *impl_data);

/// Called by impl object
void zn_gl_rendering_unit_destroy(struct zn_gl_rendering_unit *self);

#ifdef __cplusplus
}
#endif
