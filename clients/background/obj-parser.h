#ifndef ZEN_CLIENT_BACKGROUND_OBJ_PARSER_H
#define ZEN_CLIENT_BACKGROUND_OBJ_PARSER_H

#include <zukou.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "viewer.h"

struct ObjFacePoint {
  int vertex_index;
  int texture_index;
  int norm_index;
};

struct ObjObject {
  std::string name;
  std::string mtl_name;
  bool smooth_shading;
  std::vector<std::vector<ObjFacePoint>> faces;
};

struct MtlObject {
  std::string name;
  float shininess;
  glm::vec3 ambient;
  glm::vec3 diffuse;
  glm::vec3 specular;
  glm::vec3 emissive;
  float optical_density;
  float opacity;
  uint32_t illum;
};

class ObjParser
{
 public:
  ObjParser(std::string obj_path, std::string mtl_path);
  ~ObjParser();
  bool Parse();

 public:
  inline std::vector<glm::vec3> vertices();
  inline std::vector<glm::vec2> textures();
  inline std::vector<glm::vec3> norms();
  inline std::vector<ObjObject> obj_list();
  inline std::unordered_map<std::string, MtlObject> mtl_table();

 private:
  std::string obj_path_;
  std::string mtl_path_;

  std::string target_mtl_path_;
  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec2> textures_;
  std::vector<glm::vec3> norms_;
  std::vector<ObjObject> obj_list_;

  std::vector<MtlObject> mtl_list_;
  std::unordered_map<std::string, MtlObject> mtl_table_;

 private:
  void Split(std::string *str, std::vector<std::string> *tokens) const;
  void Split(
      std::string *str, std::vector<std::string> *tokens, char delimiter) const;
  std::string ExtractFileName(std::string *path);

  bool ParseMtlPath(std::vector<std::string> *tokens);
  bool ParseObjName(std::vector<std::string> *tokens);
  bool ParseVertex(std::vector<std::string> *tokens);
  bool ParseTextureVertex(std::vector<std::string> *tokens);
  bool ParseNorm(std::vector<std::string> *tokens);
  bool ParseObjMtlName(std::vector<std::string> *tokens);
  bool ParseSmoothShading(std::vector<std::string> *tokens);
  bool ParseFace(std::vector<std::string> *tokens);
  bool ParseObjLine(std::string *line);
  bool ParseObj();

  bool ParseMtlName(std::vector<std::string> *tokens);
  bool ParseMtlNs(std::vector<std::string> *tokens);
  bool ParseMtlKa(std::vector<std::string> *tokens);
  bool ParseMtlKd(std::vector<std::string> *tokens);
  bool ParseMtlKs(std::vector<std::string> *tokens);
  bool ParseMtlKe(std::vector<std::string> *tokens);
  bool ParseMtlNi(std::vector<std::string> *tokens);
  bool ParseMtlD(std::vector<std::string> *tokens);
  bool ParseMtlIllum(std::vector<std::string> *tokens);
  bool ParseMtlLine(std::string *line);
  bool ParseMtl();

  // TODO: OBJ ParserとMTL Parserを分割する
};

inline std::vector<glm::vec3>
ObjParser::vertices()
{
  return vertices_;
}

inline std::vector<glm::vec2>
ObjParser::textures()
{
  return textures_;
}

inline std::vector<glm::vec3>
ObjParser::norms()
{
  return norms_;
}

inline std::vector<ObjObject>
ObjParser::obj_list()
{
  return obj_list_;
}

inline std::unordered_map<std::string, MtlObject>
ObjParser::mtl_table()
{
  return mtl_table_;
}

#endif  //  ZEN_CLIENT_BACKGROUND_OBJ_PARSER_H
