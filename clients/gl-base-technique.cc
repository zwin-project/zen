#include "gl-base-technique.h"

#include <cstring>

#include "application.h"
#include "gl-program.h"
#include "gl-texture.h"
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
GlBaseTechnique::Bind(GlProgram *program)
{
  zgn_gl_base_technique_bind_program(proxy_, program->proxy());
}

void
GlBaseTechnique::Bind(GlVertexArray *vertex_array)
{
  zgn_gl_base_technique_bind_vertex_array(proxy_, vertex_array->proxy());
}

void
GlBaseTechnique::Bind(
    uint32_t binding, std::string name, GlTexture *texture, GLenum target)
{
  zgn_gl_base_technique_bind_texture(
      proxy_, binding, name.c_str(), texture->proxy(), target);
}

void
GlBaseTechnique::DrawArrays(GLenum mode, GLint first, GLsizei count)
{
  zgn_gl_base_technique_draw_arrays(proxy_, mode, first, count);
}

void
GlBaseTechnique::UniformVector(uint32_t location, std::string name,
    enum zgn_gl_base_technique_uniform_variable_type type, uint32_t size,
    uint32_t count, void *value)
{
  wl_array array;
  wl_array_init(&array);

  {
    size_t value_size = 4 * size * count;
    void *data = wl_array_add(&array, value_size);
    std::memcpy(data, value, value_size);
  }

  zgn_gl_base_technique_uniform_vector(
      proxy_, location, name.c_str(), type, size, count, &array);

  wl_array_release(&array);
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
