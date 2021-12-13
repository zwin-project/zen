#ifndef ZEN_CLIENT_ZPREVIEW_OBJ_VIEWER_H
#define ZEN_CLIENT_ZPREVIEW_OBJ_VIEWER_H

#include <zukou.h>

#include <string>
#include <vector>

#include "obj-object.h"
#include "viewer.h"

class ObjViewer : public ViewerInterface
{
 public:
  ObjViewer(std::string path);
  ~ObjViewer();
  bool Show();

 private:
  std::string path_;
  zukou::App *app_;
  ObjObject *object_;
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec3> norms_;
  std::vector<std::vector<ObjFacePoint>> faces_;

 private:
  void Split(std::string *str, std::vector<std::string> *tokens) const;
  void Split(
      std::string *str, std::vector<std::string> *tokens, char delimiter) const;
  bool ParseVertex(std::vector<std::string> *tokens);
  bool ParseNorm(std::vector<std::string> *tokens);
  bool ParseFace(std::vector<std::string> *tokens);
  bool ParseLine(std::string *line);
};

#endif  //  ZEN_CLIENT_ZPREVIEW_OBJ_VIEWER_H
