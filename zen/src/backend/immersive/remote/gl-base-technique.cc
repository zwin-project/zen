#include "gl-base-technique.h"

#include "gl-rendering-unit.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

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
  c_obj_ = zn_gl_base_technique_create(this);
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

}  // namespace zen::backend::immersive::remote
