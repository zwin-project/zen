#include "gl-buffer.h"

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

GlBuffer::~GlBuffer()
{
  if (c_obj_ != nullptr) {
    zn_gl_buffer_destroy(c_obj_);
  }
}

std::unique_ptr<GlBuffer>
GlBuffer::New(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  auto self = std::unique_ptr<GlBuffer>(new GlBuffer());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote GlBuffer");
    return nullptr;
  }

  return self;
}

bool
GlBuffer::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_gl_buffer_create(this);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_buffer");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlBuffer(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlBuffer");
    zn_gl_buffer_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

}  // namespace zen::backend::immersive::remote
