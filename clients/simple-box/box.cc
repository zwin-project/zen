#include "box.h"

#include <zukou.h>

Box::Box(zukou::App *app) : VirtualObject(app)
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
