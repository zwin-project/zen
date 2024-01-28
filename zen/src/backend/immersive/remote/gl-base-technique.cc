#include "gl-base-technique.hh"

#include <zwin-gl-protocol.h>

#include "gl-buffer.hh"
#include "gl-program.hh"
#include "gl-rendering-unit.hh"
#include "gl-vertex-array.hh"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_gl_base_technique_interface GlBaseTechnique::c_implementation_ = {
    GlBaseTechnique::HandleBindProgram,
    GlBaseTechnique::HandleBindVertexArray,
    GlBaseTechnique::HandleUniformVector,
    GlBaseTechnique::HandleUniformMatrix,
    GlBaseTechnique::HandleDrawArrays,
    GlBaseTechnique::HandleDrawElements,
};

GlBaseTechnique::~GlBaseTechnique()
{
  if (c_obj_ != nullptr) {
    zn_gl_base_technique_destroy(c_obj_);
  }
}

std::unique_ptr<GlBaseTechnique>
GlBaseTechnique::New(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::unique_ptr<GlRenderingUnit> &rendering_unit)
{
  auto self = std::unique_ptr<GlBaseTechnique>(new GlBaseTechnique());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel), rendering_unit)) {
    zn_error("Failed to initialize a remote GlBaseTechnique");
    return nullptr;
  }

  return self;
}

bool
GlBaseTechnique::Init(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::unique_ptr<GlRenderingUnit> &rendering_unit)
{
  c_obj_ = zn_gl_base_technique_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_base_technique");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlBaseTechnique(
      std::move(channel), rendering_unit->remote_obj()->id());
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlBaseTechnique");
    zn_gl_base_technique_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlBaseTechnique::HandleBindProgram(
    struct zn_gl_base_technique *c_obj, struct zn_gl_program *gl_program_c_obj)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  auto *gl_program = static_cast<GlProgram *>(gl_program_c_obj->impl_data);

  self->remote_obj_->BindProgram(gl_program->remote_obj()->id());
}

void
GlBaseTechnique::HandleBindVertexArray(struct zn_gl_base_technique *c_obj,
    struct zn_gl_vertex_array *gl_vertex_array_c_obj)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  auto *gl_vertex_array =
      static_cast<GlVertexArray *>(gl_vertex_array_c_obj->impl_data);

  self->remote_obj_->BindVertexArray(gl_vertex_array->remote_obj()->id());
}

void
GlBaseTechnique::HandleUniformVector(struct zn_gl_base_technique *c_obj,
    uint32_t location, const char *name, uint32_t type, uint32_t size,
    uint32_t count, void *value)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  switch (static_cast<zwn_gl_base_technique_uniform_variable_type>(type)) {
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_FLOAT:
      self->remote_obj_->GlUniformVector(
          location, name, size, count, static_cast<float *>(value));
      break;
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_UINT:
      self->remote_obj_->GlUniformVector(
          location, name, size, count, static_cast<uint32_t *>(value));
      break;
    case ZWN_GL_BASE_TECHNIQUE_UNIFORM_VARIABLE_TYPE_INT:
      self->remote_obj_->GlUniformVector(
          location, name, size, count, static_cast<int32_t *>(value));
      break;
  }
}

void
GlBaseTechnique::HandleUniformMatrix(struct zn_gl_base_technique *c_obj,
    uint32_t location, const char *name, uint32_t col, uint32_t row,
    uint32_t count, bool transpose, void *value)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  self->remote_obj_->GlUniformMatrix(
      location, name, col, row, count, transpose, static_cast<float *>(value));
}

void
GlBaseTechnique::HandleDrawArrays(struct zn_gl_base_technique *c_obj,
    uint32_t mode, int32_t first, uint32_t count)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  self->remote_obj_->GlDrawArrays(mode, first, count);
}

void
GlBaseTechnique::HandleDrawElements(struct zn_gl_base_technique *c_obj,
    uint32_t mode, uint32_t count, uint32_t type, uint64_t offset,
    struct zn_gl_buffer *gl_buffer_c_obj)
{
  auto *self = static_cast<GlBaseTechnique *>(c_obj->impl_data);

  auto *gl_buffer = static_cast<GlBuffer *>(gl_buffer_c_obj->impl_data);

  self->remote_obj_->GlDrawElements(
      mode, count, type, offset, gl_buffer->remote_obj()->id());
}

}  // namespace zen::backend::immersive::remote
