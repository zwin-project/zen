#ifndef GGN_CLIENT_BOX_H
#define GGN_CLIENT_BOX_H

#include "app.h"
#include "zgn-opengl.h"
#include "zgn-virtual-object.h"

class Box
{
 public:
  Box(App *app);
  ~Box();
  void Frame(uint32_t time);
  ZgnVirtualObject<Box> *virtual_object();
  ZgnOpenGL *opengl();

 private:
  ZgnVirtualObject<Box> *virtual_object_;
  ZgnOpenGL *opengl_;
};

inline ZgnVirtualObject<Box> *
Box::virtual_object()
{
  return virtual_object_;
}

inline ZgnOpenGL *
Box::opengl()
{
  return opengl_;
}

#endif  //  GGN_CLIENT_BOX_H
