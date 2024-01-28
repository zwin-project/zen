#pragma once

#include "zen/gl-base-technique.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_base_technique *zn_gl_base_technique_create(void *impl_data,
    const struct zn_gl_base_technique_interface *implementation);

/// Called by impl object
void zn_gl_base_technique_destroy(struct zn_gl_base_technique *self);

#ifdef __cplusplus
}
#endif
