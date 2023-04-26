#include "gl-shader.h"

#include <utility>

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

GlShader::~GlShader()
{
  if (c_obj_ != nullptr) {
    zn_gl_shader_destroy(c_obj_);
  }
}

std::unique_ptr<GlShader>
GlShader::New(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::string source, uint32_t type)

{
  auto self = std::unique_ptr<GlShader>(new GlShader());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel), std::move(source), type)) {
    zn_error("Failed to initialize a remote GlShader");
    return nullptr;
  }

  return self;
}

bool
GlShader::Init(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::string source, uint32_t type)
{
  c_obj_ = zn_gl_shader_create(this);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_shader");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlShader(
      std::move(channel), std::move(source), type);
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlShader");
    zn_gl_shader_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

}  // namespace zen::backend::immersive::remote
