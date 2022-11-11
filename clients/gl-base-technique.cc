#include "gl-base-technique.h"

#include "application.h"
#include "gl-vertex-array.h"
#include "rendering-unit.h"

namespace zen::client {

bool
GlBaseTechnique::Init(RenderingUnit *unit)
{
  proxy_ =
      zgn_gles_v32_create_gl_base_technique(app_->gles_v32(), unit->proxy());
  if (proxy_ == nullptr) {
    zn_error("Failed to create gl base technique object proxy");
    return false;
  }

  return true;
}

void
GlBaseTechnique::Bind(GlVertexArray *vertex_array)
{
  zgn_gl_base_technique_bind_vertex_array(proxy_, vertex_array->proxy());
}

void
GlBaseTechnique::DrawArrays(GLenum mode, GLint first, GLsizei count)
{
  zgn_gl_base_technique_draw_arrays(proxy_, mode, first, count);
}

GlBaseTechnique::GlBaseTechnique(Application *app) : app_(app) {}

GlBaseTechnique::~GlBaseTechnique()
{
  if (proxy_) {
    zgn_gl_base_technique_destroy(proxy_);
  }
}

std::unique_ptr<GlBaseTechnique>
CreateGlBaseTechnique(Application *app, RenderingUnit *unit)
{
  auto technique = std::make_unique<GlBaseTechnique>(app);

  if (!technique->Init(unit)) {
    return std::unique_ptr<GlBaseTechnique>();
  }

  return technique;
}

}  // namespace zen::client
