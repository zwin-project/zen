#include "gl-vertex-array.h"

#include "application.h"
#include "gl-buffer.h"

namespace zen::client {

bool
GlVertexArray::Init()
{
  proxy_ = zgn_gles_v32_create_gl_vertex_array(app_->gles_v32());
  if (proxy_ == nullptr) {
    zn_error("Failed to create gl vertex array");
    return false;
  }

  return true;
}

void
GlVertexArray::Enable(uint32_t index)
{
  zgn_gl_vertex_array_enable_vertex_attrib_array(proxy_, index);
}

void
GlVertexArray::Disable(uint32_t index)
{
  zgn_gl_vertex_array_disable_vertex_attrib_array(proxy_, index);
}

void
GlVertexArray::VertexAttribPointer(uint32_t index, int32_t size, uint32_t type,
    bool normalized, int32_t stride, uint32_t offset, GlBuffer* gl_buffer)
{
  zgn_gl_vertex_array_vertex_attrib_pointer(proxy_, index, size, type,
      normalized, stride, offset, gl_buffer->proxy());
}

GlVertexArray::GlVertexArray(Application* app) : app_(app) {}

GlVertexArray::~GlVertexArray()
{
  if (proxy_) {
    zgn_gl_vertex_array_destroy(proxy_);
  }
}

std::unique_ptr<GlVertexArray>
CreateGlVertexArray(Application* app)
{
  auto vertex_array = std::make_unique<GlVertexArray>(app);

  if (!vertex_array->Init()) {
    return std::unique_ptr<GlVertexArray>();
  }

  return vertex_array;
}

}  // namespace zen::client
