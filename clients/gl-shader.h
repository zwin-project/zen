#pragma once

#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>
#include <string>

namespace zen::client {

class Application;
class ShmPool;
class Buffer;

class GlShader
{
 public:
  DISABLE_MOVE_AND_COPY(GlShader);
  GlShader(Application *app);
  ~GlShader();

  bool Init(std::string source);

  inline zgn_gl_shader *proxy();

 private:
  Application *app_;
  zgn_gl_shader *proxy_ = nullptr;  // nonnull after initialization
  std::unique_ptr<ShmPool> pool_;
  std::unique_ptr<Buffer> buffer_;
  int fd_ = 0;
};

inline zgn_gl_shader *
GlShader::proxy()
{
  return proxy_;
}

std::unique_ptr<GlShader> CreateGlShader(Application *app, std::string source);

}  // namespace zen::client
