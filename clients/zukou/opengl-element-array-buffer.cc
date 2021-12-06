#include <zukou.h>

namespace zukou {
OpenGLElementArrayBuffer::OpenGLElementArrayBuffer(App *app, size_t size)
    : Buffer(app, size)
{
  element_array_buffer_ = zgn_opengl_create_element_array_buffer(app->opengl());
}

OpenGLElementArrayBuffer::~OpenGLElementArrayBuffer()
{
  zgn_opengl_element_array_buffer_destroy(element_array_buffer_);
}

void
OpenGLElementArrayBuffer::BufferUpdated(
    enum zgn_opengl_element_array_indices_type type)
{
  zgn_opengl_element_array_buffer_attach(
      element_array_buffer_, wl_buffer_, type);
}
}  // namespace zukou
