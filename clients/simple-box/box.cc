#include "box.h"

#include <zukou.h>

#include <glm/glm.hpp>

Box::Box(zukou::App *app, float length) : CuboidWindow(app, glm::vec3(length))
{
  component_ = new zukou::OpenGLComponent(app, this);
}

void
Box::Frame(uint32_t time)
{
  (void)time;
  static int c = 0;
  if (c < 300)
    this->NextFrame();
  else
    this->Commit();
  c++;
}
