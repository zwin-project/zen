#include "gl-vertex-array.h"

#include "gl-buffer.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_gl_vertex_array_interface GlVertexArray::c_implementation_ = {
    GlVertexArray::HandleEnableVertexAttribArray,
    GlVertexArray::HandleDisableVertexAttribArray,
    GlVertexArray::HandleVertexAttribPointer,
};

GlVertexArray::~GlVertexArray()
{
  if (c_obj_ != nullptr) {
    zn_gl_vertex_array_destroy(c_obj_);
  }
}

std::unique_ptr<GlVertexArray>
GlVertexArray::New(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  auto self = std::unique_ptr<GlVertexArray>(new GlVertexArray());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote GlVertexArray");
    return nullptr;
  }

  return self;
}

bool
GlVertexArray::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_gl_vertex_array_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_vertex_array");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlVertexArray(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlVertexArray");
    zn_gl_vertex_array_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlVertexArray::HandleEnableVertexAttribArray(
    struct zn_gl_vertex_array *c_obj, uint32_t index)
{
  auto *self = static_cast<GlVertexArray *>(c_obj->impl_data);

  self->remote_obj_->GlEnableVertexAttribArray(index);
}

void
GlVertexArray::HandleDisableVertexAttribArray(
    struct zn_gl_vertex_array *c_obj, uint32_t index)
{
  auto *self = static_cast<GlVertexArray *>(c_obj->impl_data);

  self->remote_obj_->GlDisableVertexAttribArray(index);
}

void
GlVertexArray::HandleVertexAttribPointer(struct zn_gl_vertex_array *c_obj,
    uint32_t index, int32_t size, uint32_t type, bool normalized,
    int32_t stride, uint64_t offset, zn_gl_buffer *gl_buffer_c_obj)
{
  auto *self = static_cast<GlVertexArray *>(c_obj->impl_data);

  auto *gl_buffer = static_cast<GlBuffer *>(gl_buffer_c_obj->impl_data);

  self->remote_obj_->GlVertexAttribPointer(index, size, type, normalized,
      stride, offset, gl_buffer->remote_obj()->id());
}

}  // namespace zen::backend::immersive::remote
