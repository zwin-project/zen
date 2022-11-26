#pragma once

#include <GLES3/gl32.h>
#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;
class Buffer;

class GlBuffer
{
 public:
  DISABLE_MOVE_AND_COPY(GlBuffer);
  GlBuffer() = delete;
  GlBuffer(Application* app);
  ~GlBuffer();

  bool Init();

  void Data(GLenum target, Buffer* buffer, GLenum usage);

  inline zgn_gl_buffer* proxy();

 private:
  Application* app_;
  zgn_gl_buffer* proxy_ = nullptr;  // nonnull after initialization
};

inline zgn_gl_buffer*
GlBuffer::proxy()
{
  return proxy_;
}

std::unique_ptr<GlBuffer> CreateGlBuffer(Application* app);

}  // namespace zen::client
