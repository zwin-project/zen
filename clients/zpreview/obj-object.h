#ifndef ZEN_CLIENT_ZPREVIEW_OBJ_OBJECT_H
#define ZEN_CLIENT_ZPREVIEW_OBJ_OBJECT_H

#include <zukou.h>

#include <vector>

#include "types.h"

struct ObjFacePoint {
  int vertex_index;
  int texture_index;
  int norm_index;
};

class ObjObject : public zukou::CuboidWindow
{
 public:
  ObjObject(zukou::App *app, std::vector<std::vector<ObjFacePoint>> *faces,
      std::vector<glm::vec3> *vertices, std::vector<glm::vec3> *norms,
      glm::vec3 half_size);
  ~ObjObject();
  void Configure(uint32_t serial, glm::vec3 half_size);
  virtual void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);

 private:
  zukou::OpenGLComponent *component_;
  zukou::OpenGLVertexBuffer *vertex_buffer_;
  zukou::OpenGLShaderProgram *shader_;

  glm::vec3 min_;
  glm::vec3 max_;
};

#endif  //  ZEN_CLIENT_ZPREVIEW_OBJ_OBJECT_H
