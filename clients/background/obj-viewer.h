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

class ObjViewer
{
 public:
  ObjViewer(zukou::App *app, zukou::VirtualObject *virtual_object,
      ObjParser *obj_parser);
  ~ObjViewer();
  bool Render();

 private:
  zukou::App *app_;
  zukou::VirtualObject *virtual_object_;
  ObjParser *parser_;

  std::vector<GLComponentObject> component_list_;
};

#endif  // ZEN_CLIENT_BACKGROUND_OBJ_VIEWER_H
