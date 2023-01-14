#include "gl-uniform-variable.h"

#include <zen-common.h>

/**
 * @returns 0 if two arguments represent the same uniform variable, -1 otherwise
 */
int
zwnr_gl_uniform_variable_compare(struct zwnr_gl_uniform_variable *self,
    struct zwnr_gl_uniform_variable *other)
{
  if (self->name && other->name)
    return strcmp(self->name, other->name) == 0 ? 0 : -1;

  if (self->name || other->name) return -1;

  return self->location == other->location ? 0 : -1;
}

struct zwnr_gl_uniform_variable *
zwnr_gl_uniform_variable_create(uint32_t location, const char *name,
    enum zwn_gl_base_technique_uniform_variable_type type, uint32_t col,
    uint32_t row, uint32_t count, bool transpose, void *src)
{
  struct zwnr_gl_uniform_variable *self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    goto err;
  }

  self->location = location;
  self->name = name ? strdup(name) : NULL;
  self->type = type;
  self->col = col;
  self->row = row;
  self->count = count;
  self->transpose = transpose;
  self->newly_comitted = false;
  wl_list_init(&self->link);

  size_t size = 4 * col * row * count;
  self->value = malloc(size);
  memcpy(self->value, src, size);

  return self;

err:
  return NULL;
}

void
zwnr_gl_uniform_variable_destroy(struct zwnr_gl_uniform_variable *self)
{
  wl_list_remove(&self->link);
  free(self->value);
  free(self->name);
  free(self);
}
