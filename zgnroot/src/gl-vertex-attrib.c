#include "gl-vertex-attrib.h"

#include <zen-common.h>

#include "gl-buffer.h"

/**
 * @return struct zgnr_gl_buffer* : nullable
 */
struct zgnr_gl_buffer*
zgnr_gl_vertex_attrib_get_gl_buffer(struct zgnr_gl_vertex_attrib* attrib)
{
  struct zgnr_gl_buffer_impl* gl_buffer =
      zn_weak_resource_get_user_data(&attrib->gl_buffer);

  return gl_buffer ? &gl_buffer->base : NULL;
}

struct zgnr_gl_vertex_attrib*
zgnr_gl_vertex_attrib_copy(struct zgnr_gl_vertex_attrib* src)
{
  struct zgnr_gl_vertex_attrib* dest = zgnr_gl_vertex_attrib_create(src->index);

  memcpy(dest, src, sizeof *dest);

  zn_weak_resource_init(&dest->gl_buffer);
  zn_weak_resource_link(&dest->gl_buffer, src->gl_buffer.resource);

  return dest;
}

struct zgnr_gl_vertex_attrib*
zgnr_gl_vertex_attrib_create(uint32_t index)
{
  struct zgnr_gl_vertex_attrib* self;

  self = zalloc(sizeof *self);
  if (self == NULL) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  self->index = index;
  self->enabled = false;
  self->enable_changed = false;
  self->gl_buffer_changed = false;
  wl_list_init(&self->link);

  zn_weak_resource_init(&self->gl_buffer);

  return self;

err:
  return NULL;
}

void
zgnr_gl_vertex_attrib_destroy(struct zgnr_gl_vertex_attrib* self)
{
  wl_list_remove(&self->link);
  zn_weak_resource_unlink(&self->gl_buffer);
  free(self);
}
