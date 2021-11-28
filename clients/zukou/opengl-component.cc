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

void
OpenGLComponent::Attach(OpenGLShaderProgram *shader_program)
{
  zgn_opengl_component_attach_shader_program(
      component(), shader_program->shader());
}

void
OpenGLComponent::SetMin(uint32_t min)
{
  zgn_opengl_component_set_min(component(), min);
}

void
OpenGLComponent::SetCount(uint32_t count)
{
  zgn_opengl_component_set_count(component(), count);
}

void
OpenGLComponent::SetTopology(enum zgn_opengl_topology topology)
{
  zgn_opengl_component_set_topology(component(), topology);
}

void
OpenGLComponent::AddVertexAttribute(uint32_t index, uint32_t size,
    enum zgn_opengl_vertex_attribute_type type, uint32_t normalized,
    uint32_t stride, uint32_t pointer)
{
  zgn_opengl_component_add_vertex_attribute(
      component(), index, size, type, normalized, stride, pointer);
}

}  // namespace zukou
