#include "xr-dispatcher.h"

#include <utility>

#include "virtual-object.h"
#include "zen-common/log.h"

namespace zen::backend::immersive::remote {

const zn_xr_dispatcher_interface XrDispatcher::c_implementation_ = {
    XrDispatcher::HandleGetNewVirtualObject,
    XrDispatcher::HandleDestroyVirtualObject,
};

XrDispatcher::~XrDispatcher()
{
  if (c_obj_ != nullptr) {
    zn_xr_dispatcher_destroy(c_obj_);
  }
}

std::unique_ptr<XrDispatcher>
XrDispatcher::New(std::shared_ptr<zen::remote::server::ISession> session)
{
  auto self = std::unique_ptr<XrDispatcher>(new XrDispatcher());
  if (!self) {
    zn_error("Failed to allocate memory");
    return nullptr;
  }

  if (!self->Init(std::move(session))) {
    zn_error("Failed to initialize remote XrDispatcher");
    return nullptr;
  }

  return self;
}

bool
XrDispatcher::Init(std::shared_ptr<zen::remote::server::ISession> session)
{
  c_obj_ = zn_xr_dispatcher_create(this, &c_implementation_);
  if (c_obj_ == nullptr) {
    zn_error("Failed to create zn_xr_dispatcher");
    return false;
  }

  channel_ = zen::remote::server::CreateChannel(session);
  if (!channel_) {
    zn_error("Failed to create a remote channel");
    zn_xr_dispatcher_destroy(c_obj_);
    return false;
  }

  return true;
}

zn_virtual_object *
XrDispatcher::HandleGetNewVirtualObject(zn_xr_dispatcher *c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto virtual_object = VirtualObject::New(self->channel_);
  if (!virtual_object) {
    zn_error("Failed to create a remote virtual object");
    return nullptr;
  }

  auto *virtual_object_c_obj = virtual_object->c_obj();

  self->virtual_objects_.push_back(std::move(virtual_object));

  return virtual_object_c_obj;
}

void
XrDispatcher::HandleDestroyVirtualObject(
    zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj)
{
  auto *self = static_cast<XrDispatcher *>(c_obj->impl_data);

  auto result = std::remove_if(self->virtual_objects_.begin(),
      self->virtual_objects_.end(),
      [virtual_object_c_obj](std::unique_ptr<VirtualObject> &virtual_object) {
        return virtual_object->c_obj() == virtual_object_c_obj;
      });

  self->virtual_objects_.erase(result, self->virtual_objects_.end());
}

}  // namespace zen::backend::immersive::remote
