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

class ObjViewer : public zukou::Background
{
 public:
  ObjViewer(zukou::App *app, ObjParser *obj_parser);
  ~ObjViewer();
  bool Render();

 private:
  ObjParser *parser_;

  std::vector<GLComponentObject> component_list_;
};

#endif  // ZEN_CLIENT_BACKGROUND_OBJ_VIEWER_H
