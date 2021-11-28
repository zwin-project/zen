#ifndef ZEN_CLIENT_SIMPLE_BOX_BOX_H
#define ZEN_CLIENT_SIMPLE_BOX_BOX_H

#include <zukou.h>

#include <glm/glm.hpp>

class Box : public zukou::CuboidWindow
{
 public:
  Box(zukou::App *app, float length);
  void Frame(uint32_t time);
  inline zukou::OpenGLComponent *component();
  inline zukou::OpenGLVertexBuffer *vertex_buffer();

 private:
  zukou::OpenGLComponent *component_;
  zukou::OpenGLVertexBuffer *vertex_buffer_;
  float length_;
  float rx_;
  float ry_;
};

inline zukou::OpenGLComponent *
Box::component()
{
  return component_;
}

inline zukou::OpenGLVertexBuffer *
Box::vertex_buffer()
{
  return vertex_buffer_;
}

#endif  //  ZEN_CLIENT_SIMPLE_BOX_BOX_H
