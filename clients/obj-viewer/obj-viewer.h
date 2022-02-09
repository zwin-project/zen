#ifndef ZEN_CLIENT_BACKGROUND_OBJ_VIEWER_H
#define ZEN_CLIENT_BACKGROUND_OBJ_VIEWER_H

#include <zukou.h>

#include <vector>

#include "obj-parser.h"

struct GLComponentObject {
  zukou::OpenGLComponent *component;
  zukou::OpenGLVertexBuffer *vertex_buffer;
  zukou::OpenGLShaderProgram *shader;
};

class ObjViewer : public zukou::CuboidWindow
{
 public:
  ObjViewer(zukou::App *app, ObjParser *obj_parser);
  ~ObjViewer();
  void Configure(uint32_t serial, glm::vec3 half_size, glm::quat quaternion);

  void RayButton(uint32_t serial, uint32_t time, uint32_t button,
      enum zgn_ray_button_state state);

 private:
  ObjParser *parser_;

  std::vector<GLComponentObject> component_list_;

  glm::vec3 min_;
  glm::vec3 max_;
};

#endif  // ZEN_CLIENT_BACKGROUND_OBJ_VIEWER_H
