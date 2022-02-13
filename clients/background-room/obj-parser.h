#ifndef ZEN_CLIENT_BACKGROUND_ROOM_OBJ_PARSER_H
#define ZEN_CLIENT_BACKGROUND_ROOM_OBJ_PARSER_H

#include <zukou.h>

#include <string>
#include <unordered_map>
#include <vector>

#include "types.h"

struct ObjFacePoint {
  int vertex_index;
  int texture_index;
  int norm_index;
};

typedef std::vector<ObjFacePoint> ObjFaceLine;

struct ObjObject {
  std::string name;
  bool smooth_shading;
  std::unordered_map<std::string, std::vector<Vertex>> face_table;
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
  std::string map_kd_path;
};

class ObjParser
{
 public:
  ObjParser(std::string obj_path, std::string mtl_path,
      std::string textures_dir_path);
  ~ObjParser();
  bool Parse();

 public:
  inline std::vector<ObjObject> obj_list();
  inline std::unordered_map<std::string, MtlObject> mtl_table();

  inline std::string texture_base_dir();

 private:
  std::string obj_path_;
  std::string mtl_path_;
  std::string textures_dir_path_;
  std::vector<std::string> texture_file_names_;

  std::string target_mtl_path_;
  std::string pending_mtl_name_;

  std::vector<glm::vec3> vertices_;
  std::vector<glm::vec2> texture_vertices_;
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
  bool ParseMtlMapKd(std::vector<std::string> *tokens);
  bool ParseMtlLine(std::string *line);
  bool CheckMtlTextureExist(std::string *path);
  bool ReadMtlTextureFileNames();
  bool ParseMtl();
};

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

inline std::string
ObjParser::texture_base_dir()
{
  return textures_dir_path_;
}

#endif  //  ZEN_CLIENT_BACKGROUND_ROOM_OBJ_PARSER_H
