#pragma once

#include "zgnr/gl-vertex-attrib.h"

struct zgnr_gl_vertex_attrib* zgnr_gl_vertex_attrib_copy(
    struct zgnr_gl_vertex_attrib* src);

struct zgnr_gl_vertex_attrib* zgnr_gl_vertex_attrib_create(uint32_t index);

void zgnr_gl_vertex_attrib_destroy(struct zgnr_gl_vertex_attrib* self);
