#include "gl-base-technique.h"

#include <zen-common.h>

#include "gl-vertex-array.h"
#include "rendering-unit.h"
#include "session.h"

void
znr_gl_base_technique_bind_vertex_array(struct znr_gl_base_technique* self,
    struct znr_gl_vertex_array* vertex_array)
{
  self->proxy->BindVertexArray(vertex_array->proxy->id());
}

void
znr_gl_base_technique_draw_arrays(struct znr_gl_base_technique* self,
    uint32_t mode, int32_t first, uint32_t count)
{
  self->proxy->GlDrawArrays(mode, first, count);
}

struct znr_gl_base_technique*
znr_gl_base_technique_create(
    struct znr_session* session_base, struct znr_rendering_unit* rendering_unit)
{
  auto self = new znr_gl_base_technique();
  znr_session_impl* session;

  if (self == nullptr) {
    zn_error("Failed to allocate memory");
    goto err;
  }

  session = zn_container_of(session_base, session, base);

  self->proxy = zen::remote::server::CreateGlBaseTechnique(
      session->proxy, rendering_unit->proxy->id());
  if (!self->proxy) {
    zn_error("Failed to create remote gl base technique");
    goto err_delete;
  }

  return self;

err_delete:
  delete self;

err:
  return nullptr;
}

void
znr_gl_base_technique_destroy(struct znr_gl_base_technique* self)
{
  delete self;
}
