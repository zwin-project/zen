#pragma once

#include "gl-base-technique-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlRenderingUnit;

class GlBaseTechnique
{
 public:
  DISABLE_MOVE_AND_COPY(GlBaseTechnique);
  ~GlBaseTechnique();

  static std::unique_ptr<GlBaseTechnique> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<GlRenderingUnit> &rendering_unit);

  inline std::unique_ptr<zen::remote::server::IGlBaseTechnique> &remote_obj();
  inline zn_gl_base_technique *c_obj();

 private:
  GlBaseTechnique() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel,
      std::unique_ptr<GlRenderingUnit> &rendering_unit);

  std::unique_ptr<zen::remote::server::IGlBaseTechnique> remote_obj_;

  zn_gl_base_technique *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_base_technique *
GlBaseTechnique::c_obj()
{
  return c_obj_;
}

inline std::unique_ptr<zen::remote::server::IGlBaseTechnique> &
GlBaseTechnique::remote_obj()
{
  return remote_obj_;
}

}  // namespace zen::backend::immersive::remote
