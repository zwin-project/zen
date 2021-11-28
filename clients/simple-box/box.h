#ifndef ZEN_CLIENT_SIMPLE_BOX_BOX_H
#define ZEN_CLIENT_SIMPLE_BOX_BOX_H

#include <zukou.h>

#include <glm/glm.hpp>

class Box : public zukou::CuboidWindow
{
 public:
  Box(zukou::App *app, float length);
  void Frame(uint32_t time);

 private:
  zukou::OpenGLComponent *frame_component_;
  zukou::OpenGLVertexBuffer *frame_vertex_buffer_;
  zukou::OpenGLShaderProgram *frame_shader_;
  zukou::OpenGLComponent *front_component_;
  zukou::OpenGLVertexBuffer *front_vertex_buffer_;
  zukou::OpenGLShaderProgram *front_shader_;
  zukou::OpenGLTexture *texture_;
  float length_;
  float rx_;
  float ry_;
};

#endif  //  ZEN_CLIENT_SIMPLE_BOX_BOX_H
