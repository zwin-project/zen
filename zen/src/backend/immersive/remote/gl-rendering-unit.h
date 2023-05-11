#pragma once

#include "gl-rendering-unit-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class VirtualObject;

class GlRenderingUnit
{
 public:
  DISABLE_MOVE_AND_COPY(GlRenderingUnit);
  ~GlRenderingUnit();

  static std::unique_ptr<GlRenderingUnit> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<VirtualObject> &virtual_object);

  inline std::unique_ptr<zen::remote::server::IRenderingUnit> &remote_obj();
  inline zn_gl_rendering_unit *c_obj();

 private:
  GlRenderingUnit() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<VirtualObject> &virtual_object);

  static void HandleChangeVisibility(zn_gl_rendering_unit *c_obj, bool visible);

  std::unique_ptr<zen::remote::server::IRenderingUnit> remote_obj_;

  static const zn_gl_rendering_unit_interface c_implementation_;

  zn_gl_rendering_unit *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_rendering_unit *
GlRenderingUnit::c_obj()
{
  return c_obj_;
}

inline std::unique_ptr<zen::remote::server::IRenderingUnit> &
GlRenderingUnit::remote_obj()
{
  return remote_obj_;
}

}  // namespace zen::backend::immersive::remote
