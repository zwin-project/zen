#include "virtual-object.hh"

#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_virtual_object_interface VirtualObject::c_implementation_ = {
    VirtualObject::HandleCommitted,
    VirtualObject::HandleChangeVisibility,
};

VirtualObject::~VirtualObject()
{
  if (c_obj_ != nullptr) {
    zn_virtual_object_destroy(c_obj_);
  }
}

std::unique_ptr<VirtualObject>
VirtualObject::New(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  auto self = std::unique_ptr<VirtualObject>(new VirtualObject());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(channel))) {
    zn_error("Failed to initialize a remote VirtualObject");
    return nullptr;
  }

  return self;
}

bool
VirtualObject::Init(std::shared_ptr<zen::remote::server::IChannel> channel)
{
  c_obj_ = zn_virtual_object_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create a zn_virtual_object");
    return false;
  }

  remote_obj_ = zen::remote::server::CreateVirtualObject(std::move(channel));
  if (!remote_obj_) {
    zn_error("Failed to create a remote Virtual Object");
    zn_virtual_object_destroy(c_obj_);
    c_obj_ = nullptr;
    return false;
  }

  return true;
}

void
VirtualObject::HandleCommitted(zn_virtual_object *c_obj)
{
  auto *self = static_cast<VirtualObject *>(c_obj->impl_data);

  self->remote_obj_->Commit();
}

void
VirtualObject::HandleChangeVisibility(zn_virtual_object *c_obj, bool visible)
{
  auto *self = static_cast<VirtualObject *>(c_obj->impl_data);

  self->remote_obj_->ChangeVisibility(visible);
}

}  // namespace zen::backend::immersive::remote
