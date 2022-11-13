#include "gl-vertex-array.h"

#include <zen-common.h>

#include "session.h"

struct znr_gl_vertex_array*
znr_gl_vertex_array_create(struct znr_session* session_base)
{
  auto self = new znr_gl_vertex_array();
  znr_session_impl* session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateGlVertexArray(session->proxy);
  if (!self->proxy) {
    zn_error("Failed to create remote gl vertex array");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_vertex_array_destroy(struct znr_gl_vertex_array* self)
{
  delete self;
}
