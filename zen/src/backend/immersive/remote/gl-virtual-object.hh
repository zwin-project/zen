#pragma once

#include "gl-virtual-object.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlVirtualObject
{
 public:
  DISABLE_MOVE_AND_COPY(GlVirtualObject);
  ~GlVirtualObject();

  static std::unique_ptr<GlVirtualObject> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      struct zn_xr_dispatcher *xr_dispatcher_c_obj);

  inline std::unique_ptr<zen::remote::server::IVirtualObject> &remote_obj();
  inline zn_gl_virtual_object *c_obj();

 private:
  GlVirtualObject() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel,
      struct zn_xr_dispatcher *xr_dispatcher_c_obj);

  static void HandleCommitted(zn_gl_virtual_object *c_obj);

  static void HandleChangeVisibility(zn_gl_virtual_object *c_obj, bool visible);

  std::unique_ptr<zen::remote::server::IVirtualObject> remote_obj_;

  static const zn_gl_virtual_object_interface c_implementation_;

  zn_gl_virtual_object *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_virtual_object *
GlVirtualObject::c_obj()
{
  return c_obj_;
}

inline std::unique_ptr<zen::remote::server::IVirtualObject> &
GlVirtualObject::remote_obj()
{
  return remote_obj_;
}

}  // namespace zen::backend::immersive::remote
