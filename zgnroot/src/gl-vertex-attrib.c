#include "gl-vertex-attrib.h"

struct zgnr_gl_vertex_attrib_info*
zgnr_gl_vertex_attrib_get_info(struct zgnr_gl_vertex_attrib* attrib)
{
  return &attrib->info;
}
