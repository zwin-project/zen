#ifndef ZEN_CLIENT_BACKGROUND_ROOM_OBJ_VIEWER_H
#define ZEN_CLIENT_BACKGROUND_ROOM_OBJ_VIEWER_H

#include <zukou.h>

#include <unordered_map>
#include <vector>

#include "obj-parser.h"

struct GLComponentObject {
  zukou::OpenGLComponent *component;
  zukou::OpenGLVertexBuffer *vertex_buffer;
  zukou::OpenGLShaderProgram *shader;
};

class ObjViewer : public zukou::Background
{
 public:
  ObjViewer(zukou::App *app, ObjParser *obj_parser);
  ~ObjViewer();

 private:
  ObjParser *parser_;

  std::vector<GLComponentObject> component_list_;
  std::unordered_map<std::string, zukou::OpenGLTexture *> texture_table_;

  glm::vec3 min_;
  glm::vec3 max_;
};

#endif  // ZEN_CLIENT_BACKGROUND_ROOM_OBJ_VIEWER_H
