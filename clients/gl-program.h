#pragma once

#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;

class GlProgram
{
 public:
  DISABLE_MOVE_AND_COPY(GlProgram);
  GlProgram(Application *app);
  ~GlProgram();

  bool Init();

  inline zgn_gl_program *proxy();

 private:
  Application *app_;
  zgn_gl_program *proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_gl_program *
GlProgram::proxy()
{
  return proxy_;
}

std::unique_ptr<GlProgram> CreateGlProgram(Application *app);

}  // namespace zen::client
