#pragma once

#include "gl-shader-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlShader
{
 public:
  DISABLE_MOVE_AND_COPY(GlShader);
  ~GlShader();

  static std::unique_ptr<GlShader> New(
      std::shared_ptr<zen::remote::server::IChannel> channel,
      std::string source, uint32_t type);

  inline zn_gl_shader *c_obj() const;

 private:
  GlShader() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel,
      std::string source, uint32_t type);

  std::unique_ptr<zen::remote::server::IGlShader> remote_obj_;

  zn_gl_shader *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline zn_gl_shader *
GlShader::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
