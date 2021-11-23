#include <zukou.h>

namespace zukou {
OpenGLComponent::OpenGLComponent(App *app, VirtualObject *virtual_object)
{
  app_ = app;
  component_ = zgn_opengl_create_opengl_component(
      app->opengl(), virtual_object->virtual_object());
}
}  // namespace zukou
