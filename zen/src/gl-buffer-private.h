#pragma once

#include "zen/gl-buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/// Called by impl object
struct zn_gl_buffer *zn_gl_buffer_create(void *impl_data);

/// Called by impl object
void zn_gl_buffer_destroy(struct zn_gl_buffer *self);

#ifdef __cplusplus
}
#endif
