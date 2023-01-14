#pragma once

#include "zwnr/gl-uniform-variable.h"

int zwnr_gl_uniform_variable_compare(struct zwnr_gl_uniform_variable *self,
    struct zwnr_gl_uniform_variable *other);

/**
 * @param name nullable
 * @param src must be larger than 32 * col * row bit
 */
struct zwnr_gl_uniform_variable *zwnr_gl_uniform_variable_create(
    uint32_t location, const char *name,
    enum zwn_gl_base_technique_uniform_variable_type type, uint32_t col,
    uint32_t row, uint32_t count, bool transpose, void *src);

void zwnr_gl_uniform_variable_destroy(struct zwnr_gl_uniform_variable *self);
