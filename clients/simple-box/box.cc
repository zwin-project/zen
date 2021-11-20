#include "box.h"

#include <stdio.h>
#include <time.h>

Box::Box(App *app)
{
  virtual_object_ =
      new ZgnVirtualObject<Box>(this, app->zgn_window()->compositor());
}

Box::~Box() { delete virtual_object_; }

void
Box::Frame(uint32_t time)
{
  (void)time;
  static int c = 0;

  // Run loop for 300 frames
  if (c < 300) virtual_object()->NextFrame();

  c++;
}
