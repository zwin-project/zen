#pragma once

#include <zen-common.h>
#include <zigen-gles-v32-client-protocol.h>

#include <memory>

namespace zen::client {

class Application;
class GlBuffer;

class GlVertexArray
{
 public:
  DISABLE_MOVE_AND_COPY(GlVertexArray);
  GlVertexArray(Application* app);
  ~GlVertexArray();

  bool Init();

  void Enable(uint32_t index);
  void Disable(uint32_t index);
  void VertexAttribPointer(uint32_t index, int32_t size, uint32_t type,
      bool normalized, int32_t stride, uint32_t offset, GlBuffer* gl_buffer);

  inline zgn_gl_vertex_array* proxy();

 private:
  Application* app_;
  zgn_gl_vertex_array* proxy_;
};

inline zgn_gl_vertex_array*
GlVertexArray::proxy()
{
  return proxy_;
}

std::unique_ptr<GlVertexArray> CreateGlVertexArray(Application* app);

}  // namespace zen::client
