#include "gl-base-technique.h"

#include <zen-common.h>

#include <cstring>
#include <vector>

#include "gl-buffer.h"
#include "gl-program.h"
#include "gl-texture.h"
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
znr_gl_base_technique_bind_program(
    struct znr_gl_base_technique* self, struct znr_gl_program* program)
{
  self->proxy->BindProgram(program->proxy->id());
}

void
znr_gl_base_technique_bind_texture(struct znr_gl_base_technique* self,
    uint32_t binding, const char* name, struct znr_gl_texture* texture,
    uint32_t target)
{
  self->proxy->BindTexture(binding, name, texture->proxy->id(), target);
}

void
znr_gl_base_technique_gl_uniform_vector(struct znr_gl_base_technique* self,
    uint32_t location, const char* name,
    enum zgn_gl_base_technique_uniform_variable_type type, uint32_t size,
    uint32_t count, void* value)
{
  std::string name_string = name ? name : "";
  switch (type) {
    case ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_INT:
      self->proxy->GlUniformVector(
          location, std::move(name_string), size, count, (int32_t*)value);
      break;

    case ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_UINT:
      self->proxy->GlUniformVector(
          location, std::move(name_string), size, count, (uint32_t*)value);
      break;

    case ZGN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT:
      self->proxy->GlUniformVector(
          location, std::move(name_string), size, count, (float*)value);
      break;

    default:
      zn_assert(false, "Unknown enum entry");
      break;
  }
}

void
znr_gl_base_technique_gl_uniform_matrix(struct znr_gl_base_technique* self,
    uint32_t location, const char* name, uint32_t col, uint32_t row,
    uint32_t count, bool transpose, float* value)
{
  std::string name_string = name ? name : "";
  self->proxy->GlUniformMatrix(
      location, std::move(name_string), col, row, count, transpose, value);
}

void
znr_gl_base_technique_draw_arrays(struct znr_gl_base_technique* self,
    uint32_t mode, int32_t first, uint32_t count)
{
  self->proxy->GlDrawArrays(mode, first, count);
}

void
znr_gl_base_technique_draw_elements(struct znr_gl_base_technique* self,
    uint32_t mode, uint32_t count, uint32_t type, uint64_t offset,
    struct znr_gl_buffer* element_array_buffer)
{
  self->proxy->GlDrawElements(
      mode, count, type, offset, element_array_buffer->proxy->id());
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
