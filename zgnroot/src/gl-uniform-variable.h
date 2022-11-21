#pragma once

#include "zgnr/gl-uniform-variable.h"

int zgnr_gl_uniform_variable_compare(struct zgnr_gl_uniform_variable *self,
    struct zgnr_gl_uniform_variable *other);

/**
 * @param name nullable
 * @param src must be larger than 32 * col * row bit
 */
struct zgnr_gl_uniform_variable *zgnr_gl_uniform_variable_create(
    uint32_t location, const char *name,
    enum zgn_gl_base_technique_uniform_variable_type type, uint32_t col,
    uint32_t row, uint32_t count, bool transpose, void *src);

void zgnr_gl_uniform_variable_destroy(struct zgnr_gl_uniform_variable *self);
