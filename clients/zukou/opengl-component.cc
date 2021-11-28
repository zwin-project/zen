#include <zukou.h>

namespace zukou {
OpenGLComponent::OpenGLComponent(App *app, VirtualObject *virtual_object)
{
  app_ = app;
  component_ = zgn_opengl_create_opengl_component(
      app->opengl(), virtual_object->virtual_object());
}

OpenGLComponent::~OpenGLComponent()
{
  zgn_opengl_component_destroy(component_);
}

void
OpenGLComponent::Attach(OpenGLVertexBuffer *vertex_buffer)
{
  zgn_opengl_component_attach_vertex_buffer(
      component(), vertex_buffer->vertex_buffer());
}

}  // namespace zukou
