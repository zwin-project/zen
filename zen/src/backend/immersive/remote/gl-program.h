#pragma once

#include "gl-program-private.h"
#include "zen-common/cpp-util.h"

namespace zen::backend::immersive::remote {

class GlProgram
{
 public:
  DISABLE_MOVE_AND_COPY(GlProgram);
  ~GlProgram();

  static std::unique_ptr<GlProgram> New(
      std::shared_ptr<zen::remote::server::IChannel> channel);

  inline std::unique_ptr<zen::remote::server::IGlProgram> &remote_obj();
  inline zn_gl_program *c_obj() const;

 private:
  GlProgram() = default;

  bool Init(std::shared_ptr<zen::remote::server::IChannel> channel);

  static void HandleAttachShader(
      struct zn_gl_program *c_obj, struct zn_gl_shader *gl_shader_c_obj);

  static void HandleLink(struct zn_gl_program *c_obj);

  std::unique_ptr<zen::remote::server::IGlProgram> remote_obj_;

  static const zn_gl_program_interface c_implementation_;

  zn_gl_program *c_obj_ = nullptr;  // @nonnull after Init(), @owning
};

inline std::unique_ptr<zen::remote::server::IGlProgram> &
GlProgram::remote_obj()
{
  return remote_obj_;
}

inline zn_gl_program *
GlProgram::c_obj() const
{
  return c_obj_;
}

}  // namespace zen::backend::immersive::remote
