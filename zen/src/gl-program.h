#pragma once

#include "zen/gl-program.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_program *zn_gl_program_create(
    void *impl_data, const struct zn_gl_program_interface *implementation);

/// Called by impl object
void zn_gl_program_destroy(struct zn_gl_program *self);

#ifdef __cplusplus
}
#endif
