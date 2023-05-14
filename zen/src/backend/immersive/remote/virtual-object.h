#pragma once

#include "virtual-object-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class VirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(VirtualObject);
  ~VirtualObject();

  static std::unique_ptr<VirtualObject> New(
      std::shared_ptr<zen::remote::server::IChannel> channel);

  inline std::unique_ptr<zen::remote::server::IVirtualObject> &remote_obj();
  inline zn_virtual_object *c_obj();

 private:
  VirtualObject() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  static void HandleCommitted(zn_virtual_object *c_obj);

  static void HandleChangeVisibility(zn_virtual_object *c_obj, bool visible);

  std::unique_ptr<zen::remote::server::IVirtualObject> remote_obj_;

  static const zn_virtual_object_interface c_implementation_;

  zn_virtual_object *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_virtual_object *
VirtualObject::c_obj()
{
  return c_obj_;
}

inline std::unique_ptr<zen::remote::server::IVirtualObject> &
VirtualObject::remote_obj()
{
  return remote_obj_;
}

}  // namespace zen::backend::immersive::remote
