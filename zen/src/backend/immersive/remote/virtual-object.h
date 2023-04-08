#pragma once

#include "zen-common/cpp-util.h"
#include "zen/virtual-object.h"

namespace zen::backend::immersive::remote {

class VirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(VirtualObject);
  ~VirtualObject();

  static std::unique_ptr<VirtualObject> New(
      std::shared_ptr<zen::remote::server::IChannel> channel);

  inline zn_virtual_object *c_obj();

 private:
  VirtualObject() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  std::unique_ptr<zen::remote::server::IVirtualObject> remote_obj_;

  zn_virtual_object *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_virtual_object *
VirtualObject::c_obj()
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
