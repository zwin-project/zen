#include "gl-rendering-unit.h"

#include "virtual-object.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

GlRenderingUnit::~GlRenderingUnit()
{
  if (c_obj_ != nullptr) {
    zn_gl_rendering_unit_destroy(c_obj_);
  }
}

std::unique_ptr<GlRenderingUnit>
GlRenderingUnit::New(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::unique_ptr<VirtualObject> &virtual_object)
{
  auto self = std::unique_ptr<GlRenderingUnit>(new GlRenderingUnit());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel), virtual_object)) {
    zn_error("Failed to initialize a remote GlRenderingUnit");
    return nullptr;
  }

  return self;
}

bool
GlRenderingUnit::Init(std::shared_ptr<zen::remote::server::IChannel> channel,
    std::unique_ptr<VirtualObject> &virtual_object)
{
  c_obj_ = zn_gl_rendering_unit_create(this);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_gl_rendering_unit");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateRenderingUnit(
      std::move(channel), virtual_object->remote_obj()->id());
  if (!remote_obj_) {
    zn_error("Failed to create a remote RenderingUnit");
    zn_gl_rendering_unit_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

}  // namespace zen::backend::immersive::remote
