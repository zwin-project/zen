#include "zgnr/gl-vertex-attrib.h"

#include "gl-buffer.h"

struct zgnr_gl_buffer*
zgnr_gl_vertex_attrib_get_gl_buffer(struct zgnr_gl_vertex_attrib* attrib)
{
  struct zgnr_gl_buffer_impl* gl_buffer =
      zn_weak_resource_get_user_data(&attrib->gl_buffer);

  return gl_buffer ? &gl_buffer->base : NULL;
}
