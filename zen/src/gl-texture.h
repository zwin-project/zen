#pragma once

#include "zen/gl-texture.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_texture *zn_gl_texture_create(
    void *impl_data, const struct zn_gl_texture_interface *implementation);

/// Called by impl object
void zn_gl_texture_destroy(struct zn_gl_texture *self);

#ifdef __cplusplus
}
#endif
