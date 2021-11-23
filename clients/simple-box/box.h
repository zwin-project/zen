#ifndef ZEN_CLIENT_SIMPLE_BOX_BOX_H
#define ZEN_CLIENT_SIMPLE_BOX_BOX_H

#include <zukou.h>

class Box : public zukou::VirtualObject
{
 public:
  Box(zukou::App *app);
  void Frame(uint32_t time);
  inline zukou::OpenGLComponent *component();

 private:
  zukou::OpenGLComponent *component_;
};

inline zukou::OpenGLComponent *
Box::component()
{
  return component_;
}

#endif  //  ZEN_CLIENT_SIMPLE_BOX_BOX_H
