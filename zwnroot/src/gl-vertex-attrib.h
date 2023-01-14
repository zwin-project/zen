#pragma once

#include "zwnr/gl-vertex-attrib.h"

struct zwnr_gl_vertex_attrib *zwnr_gl_vertex_attrib_copy(
    struct zwnr_gl_vertex_attrib *src);

struct zwnr_gl_vertex_attrib *zwnr_gl_vertex_attrib_create(uint32_t index);

void zwnr_gl_vertex_attrib_destroy(struct zwnr_gl_vertex_attrib *self);
