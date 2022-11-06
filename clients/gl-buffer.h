#pragma once

#include <zigen-gles-v32-client-protocol.h>

#include <memory>

#include "common.h"

namespace zen::client {

class Application;

class GlBuffer
{
 public:
  DISABLE_MOVE_AND_COPY(GlBuffer);
  GlBuffer(Application* app);
  ~GlBuffer();

  bool Init();
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
