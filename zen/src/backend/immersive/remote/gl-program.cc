#include "gl-program.h"

#include "gl-shader.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_gl_program_interface GlProgram::c_implementation_ = {
    GlProgram::HandleAttachShader,
    GlProgram::HandleLink,
};

GlProgram::~GlProgram()
{
  if (c_obj_ != nullptr) {
    zn_gl_program_destroy(c_obj_);
  }
}

std::unique_ptr<GlProgram>
GlProgram::New(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  auto self = std::unique_ptr<GlProgram>(new GlProgram());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote GlProgram");
    return nullptr;
  }

  return self;
}

bool
GlProgram::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_gl_program_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_program");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateGlProgram(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote GlProgram");
    zn_gl_program_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlProgram::HandleAttachShader(
    struct zn_gl_program *c_obj, struct zn_gl_shader *gl_shader_c_obj)
{
  auto *self = static_cast<GlProgram *>(c_obj->impl_data);

  auto *gl_shader = static_cast<GlShader *>(gl_shader_c_obj->impl_data);

  self->remote_obj_->GlAttachShader(gl_shader->remote_obj()->id());
}

void
GlProgram::HandleLink(struct zn_gl_program *c_obj)
{
  auto *self = static_cast<GlProgram *>(c_obj->impl_data);

  self->remote_obj_->GlLinkProgram();
}

}  // namespace zen::backend::immersive::remote
