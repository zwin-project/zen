#include "gl-virtual-object.hh"

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_gl_virtual_object_interface GlVirtualObject::c_implementation_ = {
    GlVirtualObject::HandleCommitted,
    GlVirtualObject::HandleChangeVisibility,
};

GlVirtualObject::~GlVirtualObject()
{
  if (c_obj_ != nullptr) {
    zn_gl_virtual_object_destroy(c_obj_);
  }
}

std::unique_ptr<GlVirtualObject>
GlVirtualObject::New(std::shared_ptr<zen::remote::server::IChannel> channel,
    struct zn_xr_dispatcher *xr_dispatcher_c_obj)
{
  auto self = std::unique_ptr<GlVirtualObject>(new GlVirtualObject());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel), xr_dispatcher_c_obj)) {
    zn_error("Failed to initialize a remote VirtualObject");
    return nullptr;
  }

  return self;
}

bool
GlVirtualObject::Init(std::shared_ptr<zen::remote::server::IChannel> channel,
    struct zn_xr_dispatcher *xr_dispatcher_c_obj)
{
  c_obj_ = zn_gl_virtual_object_create(
      this, &c_implementation_, xr_dispatcher_c_obj);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_virtual_object");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateVirtualObject(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote Virtual Object");
    zn_gl_virtual_object_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
GlVirtualObject::HandleCommitted(zn_gl_virtual_object *c_obj)
{
  auto *self = static_cast<GlVirtualObject *>(c_obj->impl_data);

  self->remote_obj_->Commit();
}

void
GlVirtualObject::HandleChangeVisibility(
    zn_gl_virtual_object *c_obj, bool visible)
{
  auto *self = static_cast<GlVirtualObject *>(c_obj->impl_data);

  self->remote_obj_->ChangeVisibility(visible);
}

}  // namespace zen::backend::immersive::remote
