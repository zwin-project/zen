#include <zukou.h>

namespace zukou {
OpenGLVertexBuffer::OpenGLVertexBuffer(App* app, size_t size)
    : Buffer(app, size)
{
  vertex_buffer_ = zgn_opengl_create_vertex_buffer(app->opengl());
}

OpenGLVertexBuffer::~OpenGLVertexBuffer()
{
  zgn_opengl_vertex_buffer_destroy(vertex_buffer_);
}

void
OpenGLVertexBuffer::BufferUpdated()
{
  zgn_opengl_vertex_buffer_attach(vertex_buffer_, wl_buffer_);
}

}  // namespace zukou
