#pragma once

#include "xr-dispatcher-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class VirtualObject;
class GlBuffer;
class GlRenderingUnit;

class XrDispatcher
{
 public:
  DISABLE_MOVE_AND_COPY(XrDispatcher);
  ~XrDispatcher();

  static std::unique_ptr<XrDispatcher> New(
      std::shared_ptr<zen::remote::server::ISession> session);

  inline zn_xr_dispatcher *c_obj();

 private:
  XrDispatcher() = default;

  bool Init(std::shared_ptr<zen::remote::server::ISession> session);

  static zn_virtual_object *HandleGetNewVirtualObject(zn_xr_dispatcher *c_obj);

  static void HandleDestroyVirtualObject(
      zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj);

  static zn_gl_rendering_unit *HandleGetNewGlRenderingUnit(
      zn_xr_dispatcher *c_obj, zn_virtual_object *virtual_object_c_obj);

  static void HandleDestroyGlRenderingUnit(
      zn_xr_dispatcher *c_obj, zn_gl_rendering_unit *gl_rendering_unit_c_obj);

  static zn_gl_buffer *HandleGetNewGlBuffer(zn_xr_dispatcher *c_obj);

  static void HandleDestroyGlBuffer(
      zn_xr_dispatcher *c_obj, struct zn_gl_buffer *gl_buffer_c_obj);

  std::shared_ptr<zen::remote::server::IChannel> channel_;

  std::vector<std::unique_ptr<VirtualObject>> virtual_objects_;

  std::vector<std::unique_ptr<GlRenderingUnit>> gl_rendering_units_;

  std::vector<std::unique_ptr<GlBuffer>> gl_buffers_;

  static const zn_xr_dispatcher_interface c_implementation_;

  zn_xr_dispatcher *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_xr_dispatcher *
XrDispatcher::c_obj()
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
